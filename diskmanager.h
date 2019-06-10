#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <QString>
#include <QList>

struct InfoAboutMountedDisks {
    QString path;
    char letter;
    long long size;
};

class DiskManager
{
public:
    DiskManager();

    int mount(const QString& url,
              const QString& password,
              long long size,
              char letter,
              unsigned int deviceId);

    int unmount(char letter);
};

#endif // DISKMANAGER_H
