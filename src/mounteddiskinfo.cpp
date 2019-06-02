#include "mounteddiskinfo.h"

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
