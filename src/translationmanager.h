#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class TranslationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages NOTIFY availableLanguagesChanged)
    Q_PROPERTY(bool useEnglishNames READ useEnglishNames WRITE setUseEnglishNames NOTIFY useEnglishNamesChanged)
    Q_PROPERTY(QString inputLanguage READ inputLanguage WRITE setInputLanguage NOTIFY inputLanguageChanged)
    Q_PROPERTY(QString outputLanguage READ outputLanguage WRITE setOutputLanguage NOTIFY outputLanguageChanged)
    Q_PROPERTY(QString translationEngine READ translationEngine WRITE setTranslationEngine NOTIFY translationEngineChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

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

    QString inputLanguage() const;
    void setInputLanguage(const QString &language);
    QString outputLanguage() const;
    void setOutputLanguage(const QString &language);
    QString translationEngine() const;
    void setTranslationEngine(const QString &engine);
    bool busy() const;

    Q_INVOKABLE void translateDetailed(const QString &text, const QString &fromLang, const QString &toLang);
    Q_INVOKABLE bool copyToClipboard(const QString &text);

Q_SIGNALS:
    void availableLanguagesChanged();
    void useEnglishNamesChanged();
    void translationError(const QString &error);
    void translationFinished(const QVariantMap &result);
    void inputLanguageChanged();
    void outputLanguageChanged();
    void translationEngineChanged();
    void busyChanged();

private:
    enum class TranslationStep {
        None,
        Translation,
        Segments
    };

    void fetchAvailableLanguages();
    QString getLanguageCode(const QString &languageName) const;
    QStringList buildTransArgs(const QString &text, const QString &fromLang, const QString &toLang, bool dictionaryOnly) const;
    QVariantList parseSegments(const QString &output) const;
    void runStep(const QStringList &args);
    void onStepFinished(int exitCode);
    void cancelPendingTranslation();
    void finishTranslation();
    void setBusy(bool busy);
    void loadSettings();
    void saveSettings();

    QStringList m_availableLanguages;
    QMap<QString, Language> m_languageMap;
    QProcess *m_process;
    QProcess *m_currentProcess = nullptr;
    TranslationStep m_step = TranslationStep::None;
    QString m_pendingText;
    QString m_pendingFrom;
    QString m_pendingTo;
    QString m_translation;
    QVariantList m_segments;
    bool m_useEnglishNames = false;
    bool m_busy = false;
    QString m_inputLanguage;
    QString m_outputLanguage;
    QString m_translationEngine;
};

#endif // TRANSLATIONMANAGER_H
