#include "translationmanager.h"
#include <KLocalizedString>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFont>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

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

bool TranslationManager::autoFocusOnLaunch() const
{
    return m_autoFocusOnLaunch;
}

void TranslationManager::setAutoFocusOnLaunch(bool value)
{
    if (m_autoFocusOnLaunch != value) {
        m_autoFocusOnLaunch = value;
        saveSettings();
        Q_EMIT autoFocusOnLaunchChanged();
    }
}

int TranslationManager::fontSize() const
{
    return m_fontSize;
}

void TranslationManager::setFontSize(int value)
{
    const int clampedValue = qBound(8, value, 32);
    if (m_fontSize != clampedValue) {
        m_fontSize = clampedValue;
        saveSettings();
        Q_EMIT fontSizeChanged();
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

QString TranslationManager::translate(const QString &text, const QString &fromLang, const QString &toLang)
{
    if (text.isEmpty()) {
        return QString();
    }

    QString fromCode = getLanguageCode(fromLang);
    QString toCode = getLanguageCode(toLang);
    
    if (toCode.isEmpty()) {
        Q_EMIT translationError(i18n("Invalid target language"));
        return QString();
    }

    QStringList args;
    if (fromLang == i18n("Auto detect")) {
        args << QStringLiteral(":%1").arg(toCode);
    } else {
        args << QStringLiteral("%1:%2").arg(fromCode, toCode);
    }
    args << text << QStringLiteral("-b");

    QProcess process;
    process.start(QStringLiteral("trans"), args);
    process.waitForFinished();

    if (process.exitCode() == 0) {
        return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    } else {
        QString error = QString::fromUtf8(process.readAllStandardError());
        Q_EMIT translationError(error);
        qDebug() << "Translation error:" << error;
        return QString();
    }
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
    m_autoFocusOnLaunch = settings.value(QStringLiteral("ui/autoFocusOnLaunch"), true).toBool();
    const int systemFontSize = QGuiApplication::font().pointSize();
    m_fontSize = qBound(8, settings.value(QStringLiteral("ui/fontSize"), systemFontSize > 0 ? systemFontSize : 10).toInt(), 32);

    qDebug() << QStringLiteral("Loaded settings - input:") << m_inputLanguage << QStringLiteral("output:") << m_outputLanguage << QStringLiteral("useEnglishNames:") << m_useEnglishNames;
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
    settings.setValue(QStringLiteral("ui/autoFocusOnLaunch"), m_autoFocusOnLaunch);
    settings.setValue(QStringLiteral("ui/fontSize"), m_fontSize);

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