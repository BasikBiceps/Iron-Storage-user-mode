#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <QString>
#include <QVariantList>

class DiskManager
{
public:
    DiskManager() = default;

    int mount(const QString& url,
              const QString& password,
              long long size,
              char letter,
              unsigned int deviceId) const;
    int unmount(char letter) const;
    void format(char letter) const;

    QVariantList requireMountedDiskInfo() const;
};

#endif // DISKMANAGER_H
