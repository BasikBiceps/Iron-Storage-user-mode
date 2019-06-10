#ifndef DISKINFORMATION_H
#define DISKINFORMATION_H
#include <QString>

struct DiskInfo {
    QString path;
    long long size;
    QString passwordHash;
};

class DiskInformationTools
{
public:
    DiskInformationTools(const QString& fileName);

    void writeIntoFile(DiskInfo& diskInfo);
    long long checkMountInfo(QString path, QString password);
    void updateFile();

private:
    QString fileName;
};

#endif // DISKINFORMATION_H
