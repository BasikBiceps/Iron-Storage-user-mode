#include "diskmanager.h"
#include "mounteddiskinfo.h"

#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QEventLoop>

#include <memory.h>

extern "C"
{
#include "IronStorage.h"
}

int DiskManager::mount(const QString& url,
                       const QString& password,
                       long long size,
                       char letter,
                       unsigned int deviceId) const
{
    QString pathForMount = "\\??\\" + url;
    pathForMount.replace("/", "\\");

    const int SIZE_STRUCT_INFO = sizeof(OPEN_FILE_INFORMATION) + pathForMount.length();
    POPEN_FILE_INFORMATION openFileInformation = reinterpret_cast<POPEN_FILE_INFORMATION>(
                new char[SIZE_STRUCT_INFO]);

    if (openFileInformation == nullptr)
    {
        qFatal("Cannot initialize memory for OPEN_FILE_INFORMATION");
        delete openFileInformation;
        return -1;
    }

    memset(openFileInformation, 0, SIZE_STRUCT_INFO);
    strcpy(openFileInformation->FileName, qUtf8Printable(pathForMount));
    openFileInformation->FileNameLength = pathForMount.length();
    openFileInformation->FileSize.QuadPart = size;
    openFileInformation->DriveLetter = letter;
    openFileInformation->PasswordLength = password.length();
    strcpy(openFileInformation->Password, qUtf8Printable(password));

    int result = IronStorageDiskMount(deviceId, openFileInformation);

    delete openFileInformation;

    return result;
}

int DiskManager::unmount(char letter) const
{
    return IronStorageDiskUnMount(letter);
}

void DiskManager::format(char letter) const
{
    QEventLoop loop;
    QThread* thread = QThread::create([&] () {
        QProcess format;
        format.start("cmd.exe");
        format.waitForStarted();
        format.waitForFinished(100);
        format.readAllStandardOutput();
        format.write(QString("format " + QString(letter) + ": \r\n").toUtf8());
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

    thread->deleteLater();
}

QVariantList DiskManager::requireMountedDiskInfo() const
{
    QVariantList result;

    for (char letter = 'A'; letter <= 'Z'; letter++)
    {
        OPEN_FILE_INFORMATION* diskInfo = IronStorageDiskStatus(letter);

        if (!diskInfo)
        {
            continue;
        }

        QString path(QByteArray(diskInfo->FileName, diskInfo->FileNameLength));
        path.remove("\\??\\");

        MountedDiskInfo diskInfoProxy(path,
                                      QString(letter),
                                      diskInfo->FileSize.QuadPart / 1024,
                                      VolumeSizeUnit::KB);

        result << QVariant::fromValue(diskInfoProxy);

        free(diskInfo);
    }

    return result;
}
