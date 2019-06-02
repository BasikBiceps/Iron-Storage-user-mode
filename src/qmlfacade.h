#ifndef QMLFACADE_H
#define QMLFACADE_H

#include <QObject>
#include <QEventLoop>
#include <QVariantList>
#include <QStringList>

#include <memory>

class QmlFacade : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QVariantList mountedDisks READ mountedDisks NOTIFY mountedDisksChanged)
    Q_PROPERTY(QStringList availableLetters READ availableLetters NOTIFY availableLettersChanged)

    enum class VolumeSizeUnit
    {
        KB, MB, GB
    };

    struct OptionsForCreateDisk
    {
        QString letter;
        bool encrypted;
        int volumeSize;
        VolumeSizeUnit volumeSizeUnit;
    };

public:
    explicit QmlFacade(QObject* parent = nullptr);

    bool busy() const;
    const QVariantList& mountedDisks() const;
    const QStringList& availableLetters() const;

    Q_INVOKABLE void updateLetters();
    Q_INVOKABLE void mount(const QString& url);
    Q_INVOKABLE void unmount(int index);
    Q_INVOKABLE void unmountAll();
    Q_INVOKABLE void createDisk(const QString& url);

    Q_INVOKABLE void passwordEntered(const QString& password);
    Q_INVOKABLE void passwordCanceled();

    Q_INVOKABLE void optionsForCreateDiskEntered(bool encrypted,
                                                 const QString& letter,
                                                 int volumeSize,
                                                 int volumeSizeUnit);
    Q_INVOKABLE void optionsForCreateDiskCanceled();

    Q_INVOKABLE void chooseDiskEntered(const QString& letter);
    Q_INVOKABLE void chooseDiskCanceled();

signals:
    void mountedDisksChanged(const QVariantList& mountedDisks);
    void availableLettersChanged(const QStringList& availableLetters);

    void passwordRequired();
    void chooseDiskRequired();
    void optionsForCreateDiskRequired();
    void error(const QString& title, const QString& text);

    void busyChanged(bool busy);

private:
    void load();
    void setBusy(bool busy);
    void format(const QString& letter);
    void pushMoutedDisk(const QString& url, const QString& letter, int volume, VolumeSizeUnit unit);

private:
    QVariantList m_mountedDisks = {};
    QStringList m_availableLetters = {};

    QEventLoop* m_passwordEventLoop = nullptr;
    QEventLoop* m_optionsForCreateDiskEventLoop = nullptr;
    QEventLoop* m_chooseDiskEventLoop = nullptr;

    QString m_choosedDisk = "";
    QString m_enteredPassword = "";
    std::unique_ptr<OptionsForCreateDisk> m_optionsForCreate = nullptr;
    bool m_busy = false;
};

#endif // QMLFACADE_H
