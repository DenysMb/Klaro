#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QProcess>
#include <QStringList>

class TranslationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages NOTIFY availableLanguagesChanged)

public:
    explicit TranslationManager(QObject *parent = nullptr);
    QStringList availableLanguages() const;

Q_SIGNALS:
    void availableLanguagesChanged();

private:
    void fetchAvailableLanguages();
    QStringList m_availableLanguages;
    QProcess *m_process;
};

#endif // TRANSLATIONMANAGER_H 