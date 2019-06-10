#include "qmlfacade.h"
#include "mounteddiskinfo.h"
#include "diskinfomodel.h"
#include "diskmanager.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <Windows.h>

#include <QGuiApplication>
#include <QLocale>

#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QStorageInfo>
#include <QCryptographicHash>

const QStringList AVAILABLE_LANGUAGES = {"en_GB", "ru_RU", "ua"};

QmlFacade::QmlFacade(QObject* parent)
    : QObject(parent)
{
    QString sysLang = QLocale::system().name();

    int index = AVAILABLE_LANGUAGES.indexOf(sysLang);
    if (index >= 0)
    {
        m_currentLanguageIndex = index;
    }

    QLocale::setDefault(AVAILABLE_LANGUAGES[m_currentLanguageIndex]);
    m_translator.load(":/translations/translate.qm");

    qApp->installTranslator(&m_translator);
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

void QmlFacade::setDiskServices(DiskManager* manager, DiskInfoModel* model)
{
    m_diskManager = manager;
    m_diskInfoModel = model;

    m_mountedDisks = m_diskManager->requireMountedDiskInfo();
    emit mountedDisksChanged(m_mountedDisks);
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

    const UserDiskInfo* userDiskInfo = m_diskInfoModel->getByPath(url);

    if (!userDiskInfo)
    {
        qCritical() << "Model not contains info about" << url;
        return;
    }

    QString password;
    QString choosedDisk;

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
        QString passwordHash = QString(hash.result());

        passwordPassed = passwordHash == userDiskInfo->passwordHash;

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

    int result = m_diskManager->mount(url,
                                      password,
                                      userDiskInfo->volume,
                                      choosedDisk.front().toLatin1(),
                                      mountedDisks().length());

    if (result != 0)
    {
        emit error("Cannot mount disk. \nPlease, load a driver in Iron Storage loader.");
        qInfo() << "Cannot mount disk, error code:" << result;
    }
    else
    {
        pushMoutedDisk(url,
                       choosedDisk,
                       userDiskInfo->volume / 1024,
                       VolumeSizeUnit::KB);
    }
}

void QmlFacade::unmount(int index)
{
    QString letter = m_mountedDisks.at(index).value<MountedDiskInfo>().letter();

    int result = m_diskManager->unmount(letter.front().toLatin1());

    if (result != 0)
    {
        emit error("Cannot umount. Please close all explorer windows");
        qInfo() << "Cannot unmount" << letter << "error code:" << result;
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

    if (QFile::exists(url))
    {
        if (!QFile::remove(url))
        {
            emit error("Cannot replace file: " + url);
            qDebug() << "Cannot replace file:" << url;
            return;
        }
    }

    long long size = static_cast<LONGLONG>(
                std::pow(1024, static_cast<int>(options.volumeSizeUnit) + 1)
                * options.volumeSize);

    int result = m_diskManager->mount(url,
                                      password,
                                      size,
                                      options.letter.front().toLatin1(),
                                      mountedDisks().length());

    if (result != 0)
    {
        emit error("Cannot create disk. \nPlease, load a driver in Iron Storage loader.");
        qInfo() << "Cannot create disk, error code:" << result;
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
        QString passwordHash = QString(hash.result());

        m_diskInfoModel->push({url, passwordHash, size});
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

void QmlFacade::changeLanguage()
{
    m_currentLanguageIndex++;

    if (m_currentLanguageIndex >= AVAILABLE_LANGUAGES.size())
    {
        m_currentLanguageIndex = 0;
    }

    qApp->removeTranslator(&m_translator);
    QLocale::setDefault(AVAILABLE_LANGUAGES[m_currentLanguageIndex]);
    m_translator.load(":/translations/translate.qm");
    qApp->installTranslator(&m_translator);

    emit languageChanged();
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
    m_diskManager->format(letter.front().toLatin1());
    setBusy(false);
}

void QmlFacade::pushMoutedDisk(const QString& url,
                               const QString& letter,
                               int volume,
                               VolumeSizeUnit unit)
{
    MountedDiskInfo mountInfo(url, letter, volume, unit);

    m_mountedDisks << QVariant::fromValue(mountInfo);
    emit mountedDisksChanged(m_mountedDisks);
}
