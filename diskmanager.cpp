#include "diskmanager.h"
#include <QDebug>
#include <QStorageInfo>

extern "C" {
#include "IronStorage.h"
}

DiskManager::DiskManager() = default;

int DiskManager::mount(const QString& url,
                       const QString& password,
                       long long size,
                       char letter,
                       unsigned int deviceId) {
    QString pathForMount = "\\??\\" + url;
    pathForMount.replace("/", "\\");

    const int SIZE_STRUCT_INFO = sizeof(OPEN_FILE_INFORMATION) + pathForMount.length();
    POPEN_FILE_INFORMATION openFileInformation = reinterpret_cast<POPEN_FILE_INFORMATION>(
                new char[SIZE_STRUCT_INFO]);

    if (openFileInformation == nullptr)
    {
   //     emit error("Error!", "Cannot initialize memory for OPEN_FILE_INFORMATION");
        qDebug() << "Cannot initialize memory for OPEN_FILE_INFORMATION";
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

int DiskManager::unmount(char letter)
{
    return IronStorageDiskUnMount(letter);
}
