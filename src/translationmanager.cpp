#include "translationmanager.h"
#include <QDebug>
#include <KLocalizedString>
#include <QRegularExpression>
#include <QSettings>
#include <QClipboard>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QDir>

namespace
{
const QStringList s_availableEngines = {QStringLiteral("auto"), QStringLiteral("google"), QStringLiteral("bing"), QStringLiteral("yandex")};
const QString s_defaultEngine = QStringLiteral("auto");
}

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    fetchAvailableLanguages();
    loadSettings();
}

QStringList TranslationManager::availableLanguages() const
{
    return m_availableLanguages;
}

bool TranslationManager::useEnglishNames() const
{
    return m_useEnglishNames;
}

void TranslationManager::setUseEnglishNames(bool value)
{
    if (m_useEnglishNames != value) {
        m_useEnglishNames = value;
        saveSettings();  // Save the setting when it changes
        Q_EMIT useEnglishNamesChanged();
        
        // Update the available languages list with new names
        QStringList currentList = m_availableLanguages;
        m_availableLanguages.clear();
        m_availableLanguages.append(i18n("Auto detect"));
        
        for (const auto &lang : m_languageMap.values()) {
            m_availableLanguages.append(value ? lang.englishName : lang.nativeName);
        }
        
        Q_EMIT availableLanguagesChanged();
    }
}

QString TranslationManager::getLanguageCode(const QString &languageName) const
{
    if (languageName == i18n("Auto detect")) {
        return QString();
    }
    
    // Find the language in our map
    for (const auto &lang : m_languageMap) {
        if (lang.englishName == languageName || lang.nativeName == languageName) {
            return lang.code;
        }
    }
    return QString();
}

bool TranslationManager::busy() const
{
    return m_busy;
}

void TranslationManager::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        Q_EMIT busyChanged();
    }
}

void TranslationManager::translateDetailed(const QString &text, const QString &fromLang, const QString &toLang)
{
    if (text.isEmpty()) {
        return;
    }

    if (getLanguageCode(toLang).isEmpty()) {
        Q_EMIT translationError(i18n("Invalid target language"));
        return;
    }

    cancelPendingTranslation();

    m_pendingText = text;
    m_pendingFrom = fromLang;
    m_pendingTo = toLang;
    m_translation.clear();
    m_segments.clear();
    m_step = TranslationStep::Translation;
    setBusy(true);

    runStep(buildTransArgs(text, fromLang, toLang, false));
}

QStringList TranslationManager::buildTransArgs(const QString &text, const QString &fromLang, const QString &toLang, bool dictionaryOnly) const
{
    QStringList args;
    args << QStringLiteral("-e") << m_translationEngine;
    if (fromLang == i18n("Auto detect")) {
        args << QStringLiteral(":%1").arg(getLanguageCode(toLang));
    } else {
        args << QStringLiteral("%1:%2").arg(getLanguageCode(fromLang), getLanguageCode(toLang));
    }
    args << text;

    if (dictionaryOnly) {
        args << QStringLiteral("-no-ansi") << QStringLiteral("-show-original") << QStringLiteral("n") << QStringLiteral("-show-original-phonetics")
             << QStringLiteral("n") << QStringLiteral("-show-translation") << QStringLiteral("n") << QStringLiteral("-show-translation-phonetics")
             << QStringLiteral("n") << QStringLiteral("-show-prompt-message") << QStringLiteral("n") << QStringLiteral("-show-languages") << QStringLiteral("n")
             << QStringLiteral("-show-original-dictionary") << QStringLiteral("n");
    } else {
        args << QStringLiteral("-b");
    }
    return args;
}

QVariantList TranslationManager::parseSegments(const QString &output) const
{
    QVariantList segments;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.startsWith(QLatin1String("    "))) {
            if (!segments.isEmpty()) {
                QVariantMap last = segments.last().toMap();
                last[QStringLiteral("alternatives")] = line.trimmed().split(QStringLiteral(", "), Qt::SkipEmptyParts);
                segments.last() = last;
            }
        } else {
            QVariantMap entry;
            entry[QStringLiteral("segment")] = line;
            entry[QStringLiteral("alternatives")] = QStringList();
            segments.append(entry);
        }
    }
    return segments;
}

void TranslationManager::runStep(const QStringList &args)
{
    m_currentProcess = new QProcess(this);
    connect(m_currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode) {
        onStepFinished(exitCode);
    });
    connect(m_currentProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        if (m_currentProcess && m_step != TranslationStep::None) {
            Q_EMIT translationError(m_currentProcess->errorString());
            finishTranslation();
        }
    });
    m_currentProcess->start(QStringLiteral("trans"), args);
}

void TranslationManager::onStepFinished(int exitCode)
{
    if (!m_currentProcess || m_step == TranslationStep::None) {
        return;
    }

    if (m_step == TranslationStep::Translation) {
        if (exitCode != 0) {
            QString error = QString::fromUtf8(m_currentProcess->readAllStandardError());
            Q_EMIT translationError(error);
            qDebug() << "Translation error:" << error;
            finishTranslation();
            return;
        }

        m_translation = QString::fromUtf8(m_currentProcess->readAllStandardOutput()).trimmed();
        if (m_translationEngine == QStringLiteral("google") || m_translationEngine == QStringLiteral("auto")) {
            m_step = TranslationStep::Segments;
            m_currentProcess->deleteLater();
            m_currentProcess = nullptr;
            runStep(buildTransArgs(m_pendingText, m_pendingFrom, m_pendingTo, true));
            return;
        }
        finishTranslation();
        return;
    }

    if (exitCode == 0) {
        m_segments = parseSegments(QString::fromUtf8(m_currentProcess->readAllStandardOutput()));
    }
    finishTranslation();
}

void TranslationManager::cancelPendingTranslation()
{
    if (m_currentProcess) {
        disconnect(m_currentProcess, nullptr, this, nullptr);
        m_currentProcess->kill();
        m_currentProcess->deleteLater();
        m_currentProcess = nullptr;
    }
    m_step = TranslationStep::None;
}

void TranslationManager::finishTranslation()
{
    if (m_currentProcess) {
        m_currentProcess->deleteLater();
        m_currentProcess = nullptr;
    }
    m_step = TranslationStep::None;
    setBusy(false);

    QVariantMap result;
    result.insert(QStringLiteral("translation"), m_translation);
    result.insert(QStringLiteral("segments"), m_segments);
    Q_EMIT translationFinished(result);
}

void TranslationManager::fetchAvailableLanguages()
{
    m_process->start(QStringLiteral("trans"), QStringList() << QStringLiteral("-list-all"));
    m_process->waitForFinished();

    if (m_process->exitCode() == 0) {
        QString output = QString::fromUtf8(m_process->readAllStandardOutput());
        QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        
        m_languageMap.clear();
        m_availableLanguages.clear();
        m_availableLanguages.append(i18n("Auto detect"));

        for (const QString &line : lines) {
            QStringList parts = line.split(QRegularExpression(QStringLiteral("\\s{2,}")), Qt::SkipEmptyParts);
            if (parts.size() == 3) {
                QString code = parts.at(0);
                QString englishName = parts.at(1);
                QString nativeName = parts.at(2);
                
                Language lang{code, englishName, nativeName};
                m_languageMap[m_useEnglishNames ? englishName : nativeName] = lang;
                m_availableLanguages.append(m_useEnglishNames ? englishName : nativeName);
            }
        }
        
        Q_EMIT availableLanguagesChanged();
    } else {
        qDebug() << "Error fetching languages:" << m_process->errorString();
        qDebug() << "Error output:" << QString::fromUtf8(m_process->readAllStandardError());
    }
}

QString TranslationManager::inputLanguage() const
{
    return m_inputLanguage;
}

void TranslationManager::setInputLanguage(const QString &language)
{
    if (m_inputLanguage != language) {
        m_inputLanguage = language;
        saveSettings();
        Q_EMIT inputLanguageChanged();
    }
}

QString TranslationManager::outputLanguage() const
{
    return m_outputLanguage;
}

void TranslationManager::setOutputLanguage(const QString &language)
{
    if (m_outputLanguage != language) {
        m_outputLanguage = language;
        saveSettings();
        Q_EMIT outputLanguageChanged();
    }
}

QString TranslationManager::translationEngine() const
{
    return m_translationEngine;
}

void TranslationManager::setTranslationEngine(const QString &engine)
{
    if (m_translationEngine != engine && s_availableEngines.contains(engine)) {
        m_translationEngine = engine;
        saveSettings();
        Q_EMIT translationEngineChanged();
    }
}

void TranslationManager::loadSettings()
{
    // Create the config directory if it doesn't exist
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configDir);
    if (!dir.exists(QStringLiteral("klaro"))) {
        dir.mkpath(QStringLiteral("klaro"));
    }
    
    // Use explicit file path to ensure Flatpak compatibility
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/klaro/settings.conf");
    QSettings settings(settingsPath, QSettings::IniFormat);
    
    qDebug() << QStringLiteral("Loading settings from:") << settingsPath;
    
    m_inputLanguage = settings.value(QStringLiteral("translation/inputLanguage"), i18n("Auto detect")).toString();
    m_outputLanguage = settings.value(QStringLiteral("translation/outputLanguage"), QStringLiteral("English")).toString();
    m_useEnglishNames = settings.value(QStringLiteral("translation/useEnglishNames"), false).toBool();
    QString engine = settings.value(QStringLiteral("translation/engine"), s_defaultEngine).toString();
    m_translationEngine = s_availableEngines.contains(engine) ? engine : s_defaultEngine;

    qDebug() << QStringLiteral("Loaded settings - input:") << m_inputLanguage << QStringLiteral("output:") << m_outputLanguage
             << QStringLiteral("useEnglishNames:") << m_useEnglishNames << QStringLiteral("engine:") << m_translationEngine;
}

void TranslationManager::saveSettings()
{
    // Create the config directory if it doesn't exist
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configDir);
    if (!dir.exists(QStringLiteral("klaro"))) {
        dir.mkpath(QStringLiteral("klaro"));
    }
    
    // Use explicit file path to ensure Flatpak compatibility
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/klaro/settings.conf");
    QSettings settings(settingsPath, QSettings::IniFormat);
    
    qDebug() << QStringLiteral("Saving settings to:") << settingsPath;
    
    settings.setValue(QStringLiteral("translation/inputLanguage"), m_inputLanguage);
    settings.setValue(QStringLiteral("translation/outputLanguage"), m_outputLanguage);
    settings.setValue(QStringLiteral("translation/useEnglishNames"), m_useEnglishNames);
    settings.setValue(QStringLiteral("translation/engine"), m_translationEngine);

    // Force sync to ensure data is written immediately
    settings.sync();
    
    qDebug() << QStringLiteral("Settings saved - input:") << m_inputLanguage << QStringLiteral("output:") << m_outputLanguage << QStringLiteral("useEnglishNames:") << m_useEnglishNames;
    
    // Check if sync was successful
    if (settings.status() != QSettings::NoError) {
        qWarning() << QStringLiteral("Failed to save settings. Status:") << settings.status();
        qWarning() << QStringLiteral("Attempted to save to:") << settingsPath;
    }
}

bool TranslationManager::copyToClipboard(const QString &text)
{
    if (text.isEmpty()) {
        return false;
    }
    
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        return false;
    }
    
    clipboard->setText(text);
    return true;
} 