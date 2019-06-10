#ifndef DISKINFOMODEL_H
#define DISKINFOMODEL_H

#include <QList>
#include <QHash>
#include <QString>

struct UserDiskInfo
{
    QString path;
    QString passwordHash;
    long long volume;
};

inline bool operator == (const UserDiskInfo& l, const UserDiskInfo& r)
{
    return l.path == r.path;
}

class DiskInfoModel
{
public:
    DiskInfoModel();
    ~DiskInfoModel();

    void push(const UserDiskInfo& userDiskInfo);
    const UserDiskInfo* getByPath(const QString& path) const;

private:
    void load();
    void save();

private:
    QString m_jsonFileName;
    QList<UserDiskInfo> m_userDisks = {};
    QHash<QString, UserDiskInfo*> m_userDisksByPath = {};
};

#endif // DISKINFOMODEL_H
