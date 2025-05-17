#include "translationmanager.h"
#include <QDebug>

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

void TranslationManager::fetchAvailableLanguages()
{
    m_process->start(QStringLiteral("trans"), QStringList() << QStringLiteral("-list-languages"));
    m_process->waitForFinished();

    if (m_process->exitCode() == 0) {
        QString output = QString::fromUtf8(m_process->readAllStandardOutput());
        m_availableLanguages = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        // Add "Auto detect" as the first option
        m_availableLanguages.prepend(QStringLiteral("auto"));
        Q_EMIT availableLanguagesChanged();
    } else {
        qDebug() << "Error fetching languages:" << m_process->errorString();
        qDebug() << "Error output:" << QString::fromUtf8(m_process->readAllStandardError());
    }
} 