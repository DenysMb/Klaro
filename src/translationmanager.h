#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QProcess>
#include <QStringList>

class TranslationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages NOTIFY availableLanguagesChanged)
    Q_PROPERTY(bool useEnglishNames READ useEnglishNames WRITE setUseEnglishNames NOTIFY useEnglishNamesChanged)

public:
    explicit TranslationManager(QObject *parent = nullptr);
    QStringList availableLanguages() const;
    bool useEnglishNames() const;
    void setUseEnglishNames(bool value);

Q_SIGNALS:
    void availableLanguagesChanged();
    void useEnglishNamesChanged();

private:
    void fetchAvailableLanguages();
    QStringList m_availableLanguages;
    QProcess *m_process;
    bool m_useEnglishNames = false;
};

#endif // TRANSLATIONMANAGER_H 