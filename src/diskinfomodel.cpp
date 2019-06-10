#include "diskinfomodel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QFile>
#include <QDebug>

DiskInfoModel::DiskInfoModel()
{
    m_jsonFileName = "user_disks_info.json";

    load();
}

DiskInfoModel::~DiskInfoModel()
{
    save();
}

void DiskInfoModel::push(const UserDiskInfo& userDiskInfo)
{
    UserDiskInfo* oldUserDiskInfo = m_userDisksByPath.value(userDiskInfo.path);
    if (!oldUserDiskInfo)
    {
        m_userDisksByPath.remove(userDiskInfo.path);
        m_userDisks.removeAll(*oldUserDiskInfo);
    }

    m_userDisks << userDiskInfo;
    m_userDisksByPath.insert(userDiskInfo.path, &m_userDisks.last());
}

const UserDiskInfo* DiskInfoModel::getByPath(const QString& path) const
{
    return m_userDisksByPath.value(path);
}

void DiskInfoModel::load()
{
    QFile file(m_jsonFileName);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    if (!file.isOpen())
    {
        return;
    }

    QJsonDocument json = QJsonDocument::fromJson(file.readAll());
    QJsonArray disks = json.array();

    for (const QJsonValue& userDisk : disks)
    {
        QString path = userDisk["path"].toString();

        if (!QFile::exists(path))
        {
            qInfo() << "File missed:" << path;
            continue;
        }

        UserDiskInfo userDiskInfo;
        userDiskInfo.path = path;
        userDiskInfo.passwordHash = userDisk["passwordHash"].toString();
        userDiskInfo.volume = static_cast<long long>(userDisk["volume"].toDouble());

        push(userDiskInfo);
    }

    file.close();
}

void DiskInfoModel::save()
{
    QJsonArray disks;

    for (const UserDiskInfo& userDiskInfo : m_userDisks)
    {
        QJsonObject userDisk;

        userDisk["path"] = userDiskInfo.path;
        userDisk["passwordHash"] = userDiskInfo.passwordHash;
        userDisk["volume"] = static_cast<double>(userDiskInfo.volume);

        disks.append(userDisk);
    }

    QJsonDocument json(disks);

    QFile file(m_jsonFileName);

    file.open(QIODevice::WriteOnly | QIODevice::Text);

    if (!file.isOpen())
    {
        qFatal("Cannot open user disks file for writing");
    }

    file.write(json.toJson());
}
