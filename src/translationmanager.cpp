#include "translationmanager.h"
#include <QDebug>
#include <KLocalizedString>
#include <QRegularExpression>

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    fetchAvailableLanguages();
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