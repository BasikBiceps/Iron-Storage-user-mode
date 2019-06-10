#include "mounteddiskinfo.h"

MountedDiskInfo::MountedDiskInfo(const QString& url,
                                 const QString& letter,
                                 int volume,
                                 VolumeSizeUnit unit)
    : m_path(url)
    , m_letter(letter)
{
    static const QString UNITS[] = {"KB", "MB", "GB"};

    while (unit != VolumeSizeUnit::GB && volume >= 1024)
    {
        volume /= 1024;
        unit = static_cast<VolumeSizeUnit>(static_cast<int>(unit)+ 1);
    }

    setVolume(QString::number(volume) + UNITS[static_cast<int>(unit)]);
}

const QString &MountedDiskInfo::letter() const
{
    return m_letter;
}

const QString &MountedDiskInfo::path() const
{
    return m_path;
}

const QString &MountedDiskInfo::volume() const
{
    return m_volume;
}

void MountedDiskInfo::setLetter(const QString& letter)
{
    m_letter = letter;
}

void MountedDiskInfo::setPath(const QString& path)
{
    m_path = path;
}

void MountedDiskInfo::setVolume(const QString& volume)
{
    m_volume = volume;
}
