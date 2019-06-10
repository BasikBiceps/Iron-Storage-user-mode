#include "diskinformationtools.h"
#include <QFile>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

DiskInformationTools::DiskInformationTools(const QString& fileName)
{
    this->fileName = fileName;
}


void DiskInformationTools::writeIntoFile(DiskInfo &diskInfo)
{
    QFile file;

    file.setFileName(fileName);
    file.open(QIODevice::ReadWrite | QIODevice::Text);

    if (!file.isOpen()) {
        qDebug() << "Error with open file!";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray array = (doc.object())["Disks"].toArray();

    {
        QJsonObject temp;
        temp["Path"] = diskInfo.path;
        temp["Size"] = diskInfo.size;
        temp["Password"] = diskInfo.passwordHash;
        array.append((temp));
    }

    QJsonObject obj;
    obj["Disks"] = array;

    file.resize(0);

    QJsonDocument newDoc(obj);
    file.write(newDoc.toJson());

    file.close();
}

long long DiskInformationTools::checkMountInfo(QString path, QString password)
{
    QFile file;

    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    if (!file.isOpen()) {
        qDebug() << "Error with open file!";
        return 0;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray array = (doc.object())["Disks"].toArray();

    for (const auto obj : array) {
        if (obj.toObject().value("Path") == path && obj.toObject().value("Password") == password) {
            file.close();
            return static_cast<long long>(obj.toObject().value("Size").toDouble());
        }
    }

    file.close();

    return 0;
}

void DiskInformationTools::updateFile()
{
    QFile file;

    file.setFileName(fileName);
    file.open(QIODevice::ReadWrite | QIODevice::Text);

    if (!file.isOpen()) {
        qDebug() << "Error with open file!";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray array = (doc.object())["Disks"].toArray();
    QJsonArray newArray;

    for (const auto obj : array) {
        bool exist = QFile::exists(obj.toObject().value("Path").toString());
        if (exist) {
            newArray.append(obj.toObject());
        }
    }

    QJsonObject obj;
    obj["Disks"] = newArray;

    file.resize(0);

    QJsonDocument newDoc(obj);
    file.write(newDoc.toJson());

    file.close();
}
