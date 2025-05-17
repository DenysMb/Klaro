#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QMap>

class TranslationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages NOTIFY availableLanguagesChanged)
    Q_PROPERTY(bool useEnglishNames READ useEnglishNames WRITE setUseEnglishNames NOTIFY useEnglishNamesChanged)

public:
    struct Language {
        QString code;
        QString englishName;
        QString nativeName;
    };

    explicit TranslationManager(QObject *parent = nullptr);
    QStringList availableLanguages() const;
    bool useEnglishNames() const;
    void setUseEnglishNames(bool value);

    Q_INVOKABLE QString translate(const QString &text, const QString &fromLang, const QString &toLang);

Q_SIGNALS:
    void availableLanguagesChanged();
    void useEnglishNamesChanged();
    void translationError(const QString &error);

private:
    void fetchAvailableLanguages();
    QString getLanguageCode(const QString &languageName) const;
    QStringList m_availableLanguages;
    QMap<QString, Language> m_languageMap; // Maps display name to Language struct
    QProcess *m_process;
    bool m_useEnglishNames = false;
};

#endif // TRANSLATIONMANAGER_H 