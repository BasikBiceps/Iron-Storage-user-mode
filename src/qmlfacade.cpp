#include "qmlfacade.h"
#include "mounteddiskinfo.h"

extern "C" {
#include <filedisk.h>
}

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <Windows.h>

#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QStorageInfo>
#include <QFile>

QmlFacade::QmlFacade(QObject* parent)
    : QObject(parent)
{
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

    m_enteredPassword.clear();

    updateLetters();
    emit chooseDiskRequired();

    QEventLoop loop;
    m_chooseDiskEventLoop = &loop;
    loop.exec();

    if (m_choosedDisk.isEmpty())
    {
        qDebug() << "Disk is not selected!";
        return;
    }

    int result = 0; // TODO: mount

    if (result != 0)
    {
        emit errot("Error!", "Cannot mount disk: " + url);
        qDebug() << "Cannot mount disk, error code:" << result;
    }
    else
    {
        pushMoutedDisk(url, m_choosedDisk, 0, VolumeSizeUnit::KB);
    }

    m_choosedDisk.clear();
}

void QmlFacade::unmount(int index)
{
    QString letter = m_mountedDisks.at(index).value<MountedDiskInfo>().letter();

    int result = FileDiskUmount(letter.toLower().front().toLatin1());

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

    const BOOLEAN CD_IMAGE = FALSE;

    QString pathForCreation = "\\??\\" + url;
    pathForCreation.replace("/", "\\");

    const int SIZE_STRUCT_INFO = sizeof(OPEN_FILE_INFORMATION) + pathForCreation.length();
    POPEN_FILE_INFORMATION openFileInformation = reinterpret_cast<POPEN_FILE_INFORMATION>(
                new char[SIZE_STRUCT_INFO]);

    if (openFileInformation == nullptr)
    {
        emit error("Error!", "Cannot initialize memory for OPEN_FILE_INFORMATION");
        qDebug() << "Cannot initialize memory for OPEN_FILE_INFORMATION";
        return;
    }

    memset(openFileInformation, 0, SIZE_STRUCT_INFO);
    strcpy(openFileInformation->FileName, qUtf8Printable(pathForCreation));

    openFileInformation->FileNameLength = pathForCreation.length();

    openFileInformation->FileSize.QuadPart = static_cast<LONGLONG>(
                std::pow(1024, static_cast<int>(m_optionsForCreate->volumeSizeUnit) + 1)
                * m_optionsForCreate->volumeSize);

    openFileInformation->DriveLetter = m_optionsForCreate->letter[0].toLatin1();

    int result = FileDiskMount(mountedDisks().length(), openFileInformation, CD_IMAGE);

    if (result != 0)
    {
        emit error("Error!", "Cannot create file disk");
        qDebug() << "Cannot create file disk, error code:" << result;
    }
    else
    {
       format(m_optionsForCreate->letter);
       pushMoutedDisk(url,
                      m_optionsForCreate->letter,
                      m_optionsForCreate->volumeSize,
                      m_optionsForCreate->volumeSizeUnit);
    }

    delete openFileInformation;
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

void QmlFacade::chooseDiskEntered(const QString& letter)
{
    m_choosedDisk = letter;

    if (m_chooseDiskEventLoop)
    {
        m_chooseDiskEventLoop->quit();
        m_chooseDiskEventLoop = nullptr;
    }
}

void QmlFacade::chooseDiskCanceled()
{
    qDebug() << "Choose disk canceled";

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
            pushMoutedDisk("", QString(info.displayName().front()), 0, VolumeSizeUnit::KB);
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
