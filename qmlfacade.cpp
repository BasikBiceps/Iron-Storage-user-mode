#include "qmlfacade.h"
#include "mounteddiskinfo.h"

#include <QDebug>
#include <QStorageInfo>

QmlFacade::QmlFacade(QObject* parent)
    : QObject(parent)
{
}

const QVariantList& QmlFacade::mountedDisks() const
{
    return m_mountedDisks;
}

const QStringList& QmlFacade::availableLetters() const
{
    return m_availableLetters;
}

void QmlFacade::updateLetters()
{
    m_availableLetters.reserve('Z' - 'A');

    for (char l = 'A'; l <= 'Z'; l++)
    {
        m_availableLetters << QString(l);
    }

    for (const QStorageInfo& info : QStorageInfo::mountedVolumes())
    {
        m_availableLetters.removeOne(QString(info.rootPath()[0]));
    }

    emit availableLettersChanged(m_availableLetters);
}

void QmlFacade::mount(const QString& url)
{
    qDebug() << "Mount required:" << url;

    for (auto const& disk : m_mountedDisks)
    {
        MountedDiskInfo info = disk.value<MountedDiskInfo>();
        if (url == info.path())
        {
            emit error("Error!", "Disk already mounted");
            return;
        }
    }

    // TODO: check url and file

    bool passwordPassed = false;
    int tryingCount = 0;
    while (!passwordPassed)
    {
        emit passwordRequired();

        QEventLoop loop;
        m_passwordEventLoop = &loop;
        loop.exec();

        if (m_enteredPassword.isEmpty())
        {
            qDebug() << "Password is not entered";
            return;
        }

        passwordPassed = m_enteredPassword == "abc";

        if (++tryingCount >= 3)
        {
            qDebug() << "Input password limit";
            return;
        }
    }

    // TODO: mount file
    MountedDiskInfo mountInfo;

    mountInfo.setPath(url);
    mountInfo.setLetter("C"); // TODO: set data from mount info
    mountInfo.setVolume("567MB"); // TODO: set data from mount info

    m_mountedDisks << QVariant::fromValue(mountInfo);
    emit mountedDisksChanged(m_mountedDisks);

    m_enteredPassword.clear();
}

void QmlFacade::unmount(int index)
{
    QString path = m_mountedDisks.at(index).value<MountedDiskInfo>().path();

    // TODO: unmount

    m_mountedDisks.removeAt(index);
    mountedDisksChanged(m_mountedDisks);
}

void QmlFacade::unmountAll()
{
    while (!m_mountedDisks.empty())
    {
        unmount(0);
    }
}

void QmlFacade::createDisk(const QString &url)
{
    qDebug() << "Create disk:" << url;
    emit optionsForCreateDiskRequired();

    QEventLoop loop;
    m_optionsForCreateDiskEventLoop = &loop;
    loop.exec();

    if (!m_optionsForCreate)
    {
        qDebug() <<"Options for create broken";
        return;
    }

    emit passwordRequired();

    m_passwordEventLoop = &loop;
    loop.exec();

    if (m_enteredPassword.isEmpty())
    {
        qDebug() << "Password is not entered";
        return;
    }

    // TODO: create disk

    m_optionsForCreate.reset();
}

void QmlFacade::passwordEntered(const QString& password)
{
    qDebug() << "Password entered:" << password;
    m_enteredPassword = password;
    if (m_passwordEventLoop)
    {
        m_passwordEventLoop->quit();
        m_passwordEventLoop = nullptr;
    }
}

void QmlFacade::passwordCanceled()
{
    qDebug() << "Password canceled";
    if (m_passwordEventLoop)
    {
        m_passwordEventLoop->quit();
        m_passwordEventLoop = nullptr;
    }
}

void QmlFacade::optionsForCreateDiskEntered(bool encrypted,
                                            const QString& letter,
                                            int volumeSize,
                                            int volumeSizeUnit)
{
    qDebug() << "Options for create disk entered"
             << encrypted
             << letter
             << volumeSize
             << volumeSizeUnit;

    m_optionsForCreate.reset(new OptionsForCreateDisk{
                                 letter,
                                 encrypted,
                                 volumeSize,
                                 static_cast<VolumeSizeUnit>(volumeSizeUnit)
                             });

    if (m_optionsForCreateDiskEventLoop)
    {
        m_optionsForCreateDiskEventLoop->quit();
        m_optionsForCreateDiskEventLoop = nullptr;
    }
}

void QmlFacade::optionsForCreateDiskCanceled()
{
    qDebug() << "Options for create disk canceled";

    if (m_optionsForCreateDiskEventLoop)
    {
        m_optionsForCreateDiskEventLoop->quit();
        m_optionsForCreateDiskEventLoop = nullptr;
    }
}
