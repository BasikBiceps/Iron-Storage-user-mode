#include "qmlfacade.h"
#include "mounteddiskinfo.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>

extern "C" {
#include "IronStorage.h"
}

#include <Windows.h>

#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QStorageInfo>
#include <QCryptographicHash>

QmlFacade::QmlFacade(QObject* parent)
    : QObject(parent)
{
    m_diskInfo.updateFile();
    load();
}

bool QmlFacade::busy() const
{
    return m_busy;
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

    QString password;
    QString choosedDisk;
    long long size = 0;

    bool passwordPassed = false;
    int tryingCount = 0;
    while (!passwordPassed)
    {
        emit passwordRequired();

        QEventLoop loop;
        m_enteredPassword = &password;
        m_passwordEventLoop = &loop;
        loop.exec();

        if (password.isEmpty())
        {
            qDebug() << "Password is not entered";
            return;
        }

        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(password.toUtf8());
        QString hashPassword = QString(hash.result());

        size = m_diskInfo.checkMountInfo(url, hashPassword);
        passwordPassed = size;

        if (++tryingCount >= 3)
        {
            qDebug() << "Input password limit";
            return;
        }
    }

    updateLetters();
    emit chooseDiskRequired();

    QEventLoop loop;
    m_choosedDisk = &choosedDisk;
    m_chooseDiskEventLoop = &loop;
    loop.exec();

    if (choosedDisk.isEmpty())
    {
        qDebug() << "Disk is not selected!";
        return;
    }

    int result = m_diskManager.mount(url,password,size,choosedDisk[0].toLatin1(),mountedDisks().length());

    if (result != 0)
    {
        emit error("Error!", "Cannot mount disk. \nPlease, load a driver in Iron Storage loader.");
        qDebug() << "Cannot mount disk, error code:" << result;
    }
    else
    {
        pushMoutedDisk(url,
                       choosedDisk,
                       size / 1024,
                       VolumeSizeUnit::KB);
    }
}

void QmlFacade::unmount(int index)
{
    QString letter = m_mountedDisks.at(index).value<MountedDiskInfo>().letter();

    int result = m_diskManager.unmount(letter.front().toLatin1());

    if (result != 0)
    {
        emit error("Error!", "Cannot umount. Please close all explorer windows");
        qDebug() << "Cannot unmount" << letter << "error code:" << result;
    }
    else
    {
        m_mountedDisks.removeAt(index);
        mountedDisksChanged(m_mountedDisks);
    }
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

    OptionsForCreateDisk options;
    QString password;

    QEventLoop loop;
    m_optionsForCreate = &options;
    m_optionsForCreateDiskEventLoop = &loop;
    loop.exec();

    if (options.letter.isEmpty())
    {
        qDebug() <<"Options for create broken";
        return;
    }

    emit passwordRequired();

    m_enteredPassword = &password;
    m_passwordEventLoop = &loop;
    loop.exec();

    if (password.isEmpty())
    {
        qDebug() << "Password is not entered";
        return;
    }

    QFile file;
    file.setFileName(url);
    if (file.exists())
    {
        if (!file.remove())
        {
            emit error("Error!", "Cannot replace file: " + url);
            qDebug() << "Cannot replace file:" << url;
            return;
        }
    }
    long long size = static_cast<LONGLONG>(std::pow(1024, static_cast<int>(options.volumeSizeUnit) + 1)
                                           * options.volumeSize);

    int result = m_diskManager.mount(url,password,size,options.letter[0].toLatin1(),mountedDisks().length());

    if (result != 0)
    {
        emit error("Error!", "Cannot create disk. \nPlease, load a driver in Iron Storage loader.");
        qDebug() << "Cannot create disk, error code:" << result;
    }
    else
    {
        format(options.letter);
        pushMoutedDisk(url,
                       options.letter,
                       options.volumeSize,
                       options.volumeSizeUnit);

        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(qUtf8Printable(password), password.length());
        QString hashPassword = QString(hash.result());

        DiskInfo diskInfo;
        diskInfo.path = url;
        diskInfo.size = size;
        diskInfo.passwordHash = hashPassword;

        m_diskInfo.writeIntoFile(diskInfo);
    }
}

void QmlFacade::passwordEntered(const QString& password)
{
    qDebug() << "Password entered:" << password;

    if (m_enteredPassword)
    {
        *m_enteredPassword = password;
        m_enteredPassword = nullptr;
    }

    if (m_passwordEventLoop)
    {
        m_passwordEventLoop->quit();
        m_passwordEventLoop = nullptr;
    }
}

void QmlFacade::passwordCanceled()
{
    qDebug() << "Password canceled";

    m_enteredPassword = nullptr;

    if (m_passwordEventLoop)
    {
        m_passwordEventLoop->quit();
        m_passwordEventLoop = nullptr;
    }
}

void QmlFacade::optionsForCreateDiskEntered(const QString& letter, int volumeSize, int volumeSizeUnit)
{
    qDebug() << "Options for create disk entered"
             << letter
             << volumeSize
             << volumeSizeUnit;

    if (m_optionsForCreate)
    {
        m_optionsForCreate->letter = letter;
        m_optionsForCreate->volumeSize = volumeSize;
        m_optionsForCreate->volumeSizeUnit = static_cast<VolumeSizeUnit>(volumeSizeUnit);
        m_optionsForCreate = nullptr;
    }

    if (m_optionsForCreateDiskEventLoop)
    {
        m_optionsForCreateDiskEventLoop->quit();
        m_optionsForCreateDiskEventLoop = nullptr;
    }
}

void QmlFacade::optionsForCreateDiskCanceled()
{
    qDebug() << "Options for create disk canceled";

    m_optionsForCreate = nullptr;

    if (m_optionsForCreateDiskEventLoop)
    {
        m_optionsForCreateDiskEventLoop->quit();
        m_optionsForCreateDiskEventLoop = nullptr;
    }
}

void QmlFacade::chooseDiskEntered(const QString& letter)
{
    if (m_choosedDisk)
    {
        *m_choosedDisk = letter;
        m_choosedDisk = nullptr;
    }

    if (m_chooseDiskEventLoop)
    {
        m_chooseDiskEventLoop->quit();
        m_chooseDiskEventLoop = nullptr;
    }
}

void QmlFacade::chooseDiskCanceled()
{
    qDebug() << "Choose disk canceled";

    m_choosedDisk = nullptr;

    if (m_chooseDiskEventLoop)
    {
        m_chooseDiskEventLoop->quit();
        m_chooseDiskEventLoop = nullptr;
    }
}

void QmlFacade::load()
{
    for (const QStorageInfo& info : QStorageInfo::mountedVolumes())
    {
        if (!info.isValid())
        {
            OPEN_FILE_INFORMATION* infoAboutDisk = IronStorageDiskStatus(info.displayName().front().toLatin1());

            QString path(QByteArray(infoAboutDisk->FileName, infoAboutDisk->FileNameLength));
            path.remove("\\??\\");
            pushMoutedDisk(path, QString(info.displayName().front()),
                           infoAboutDisk->FileSize.QuadPart / 1024,
                           VolumeSizeUnit::KB);
            free(infoAboutDisk);
        }
    }

}

void QmlFacade::setBusy(bool busy)
{
    if (m_busy == busy)
    {
        return;
    }

    m_busy = busy;
    emit busyChanged(m_busy);
}

void QmlFacade::format(const QString& letter)
{
    setBusy(true);
    QEventLoop loop;
    QThread* thread = QThread::create([&] () {
        QProcess format;
        format.start("cmd.exe");
        format.waitForStarted();
        format.waitForFinished(100);
        format.readAllStandardOutput();
        format.write(QString("format " + letter + ": \r\n").toUtf8());
        format.readAllStandardOutput();
        format.write(QString("y \r\n").toUtf8());
        format.readAllStandardOutput();
        format.write(QString("\r\n").toUtf8());
        format.closeWriteChannel();
        format.waitForFinished(-1);
        format.readAllStandardOutput();
        loop.quit();
    });
    thread->start();
    loop.exec();
    setBusy(false);
}

void QmlFacade::pushMoutedDisk(const QString& url, const QString& letter, int volume, VolumeSizeUnit unit)
{
    static const QString UNITS[] = {"KB", "MB", "GB"};

    while (unit != VolumeSizeUnit::GB && volume >= 1024)
    {
        volume /= 1024;
        unit = static_cast<VolumeSizeUnit>(static_cast<int>(unit)+ 1);
    }

    MountedDiskInfo mountInfo;

    mountInfo.setPath(url);
    mountInfo.setLetter(letter);
    mountInfo.setVolume(QString::number(volume) + UNITS[static_cast<int>(unit)]);

    m_mountedDisks << QVariant::fromValue(mountInfo);
    emit mountedDisksChanged(m_mountedDisks);
}
