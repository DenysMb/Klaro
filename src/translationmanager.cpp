#include "translationmanager.h"
#include <QDebug>
#include <KLocalizedString>
#include <QRegularExpression>
#include <QSettings>
#include <QClipboard>
#include <QGuiApplication>

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
    QSettings settings;
    m_inputLanguage = settings.value(QStringLiteral("translation/inputLanguage"), i18n("Auto detect")).toString();
    m_outputLanguage = settings.value(QStringLiteral("translation/outputLanguage"), QStringLiteral("English")).toString();
    m_useEnglishNames = settings.value(QStringLiteral("translation/useEnglishNames"), false).toBool();
}

void TranslationManager::saveSettings()
{
    QSettings settings;
    settings.setValue(QStringLiteral("translation/inputLanguage"), m_inputLanguage);
    settings.setValue(QStringLiteral("translation/outputLanguage"), m_outputLanguage);
    settings.setValue(QStringLiteral("translation/useEnglishNames"), m_useEnglishNames);
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