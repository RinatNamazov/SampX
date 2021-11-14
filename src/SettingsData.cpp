/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SettingsData.cpp
 *  DESCRIPTION:    Read/write config file 
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#include <QFile>
#include <QStandardPaths>
#include <QTextCodec>

#include <cstring>

#include "SettingsData.hpp"

#include "utils.hpp"

// Todo: Redo the file structure. Fix the deletion of some elements.

constexpr char RINWARES_SAMPX_TAG[14]
    = {'R', 'I', 'N', 'W', 'A', 'R', 'E', 'S', '_', 'S', 'A', 'M', 'P', 'X'};

constexpr quint32 SAMPX_FILE_VERSION = 1;

template<typename T>
T readNumber(QFile& file)
{
    T number;
    file.read(reinterpret_cast<char*>(&number), sizeof(number));
    return number;
}

template<typename T>
QString readString(QFile& file)
{
    QTextCodec* codec{QTextCodec::codecForName("cp1251")};
    T           length{readNumber<T>(file)};
    if (length > 0) {
        return codec->toUnicode(file.read(length));
    }
    return "";
}

template<typename T>
void writeNumber(QFile& file, T number)
{
    file.write(reinterpret_cast<char*>(&number), sizeof(number));
}

template<typename T>
void writeString(QFile& file, QString& str)
{
    QTextCodec* codec{QTextCodec::codecForName("cp1251")};
    T           length{static_cast<T>(str.length())};
    writeNumber(file, length);
    if (length > 0) {
        file.write(codec->fromUnicode(str).toStdString().c_str(), length);
    }
}

bool SettingsData::load(const QString& filePath)
{
    filePath_ = filePath;

    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    FileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (std::memcmp(header.tag, RINWARES_SAMPX_TAG, sizeof(RINWARES_SAMPX_TAG)) != 0) {
        return false;
    }

    if (header.version != SAMPX_FILE_VERSION) {
        return false;
    }

    auto readUint16   = [&file]() { return readNumber<quint16>(file); };
    auto readUint32   = [&file]() { return readNumber<quint32>(file); };
    auto readString8  = [&file]() { return readString<quint8>(file); };
    auto readString16 = [&file]() { return readString<quint16>(file); };
    auto readString32 = [&file]() { return readString<quint32>(file); };

    proxys_.reserve(header.proxyCount);
    for (quint32 i{0}; i < header.proxyCount; ++i) {
        proxys_.append({readString8(), readString8()});
    }

    adapters_.reserve(header.adapterCount);
    for (quint32 i{0}; i < header.adapterCount; ++i) {
        adapters_.append(readString8());
    }

    gameDirs_.reserve(header.gameDirCount);
    for (quint32 i{0}; i < header.gameDirCount; ++i) {
        gameDirs_.append(readString16());
    }

    gameExecutables_.reserve(header.gameExeCount);
    for (quint32 i{0}; i < header.gameExeCount; ++i) {
        gameExecutables_.append(readString8());
    }

    sampVersions_.reserve(header.sampCount);
    for (quint32 i{0}; i < header.sampCount; ++i) {
        sampVersions_.append({readString8(), readString8()});
    }

    groups_.reserve(header.groupCount);
    for (quint32 i{0}; i < header.groupCount; ++i) {
        groups_.append(readString8());
    }

    servers_.reserve(header.serverCount);
    for (quint32 i{0}; i < header.serverCount; ++i) {
        Server server;

        server.group   = readUint32();
        server.address = readString8();
        quint16 port{readUint16()};
        server.address += QString(":%1").arg(port);
        server.password = readString8();
        server.rcon     = readString8();
        server.profile  = readUint32();

        servers_.append(server);
    }

    profiles_.reserve(header.profileCount);
    for (quint32 i{0}; i < header.profileCount; ++i) {
        Profile profile;

        profile.name     = readString8();
        profile.nickname = readString8();
        profile.proxy    = readUint32();
        profile.adapter  = readUint32();
        profile.gameDir  = readUint32();
        profile.gameExe  = readUint32();
        profile.samp     = readUint32();
        profile.comment  = readString32();

        profiles_.append(profile);
    }

    file.close();

    return true;
}

bool SettingsData::save(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    FileHeader header;
    memcpy(&header.tag, RINWARES_SAMPX_TAG, sizeof(RINWARES_SAMPX_TAG));
    header.version = SAMPX_FILE_VERSION;

    header.proxyCount   = getProxyCount();
    header.adapterCount = getAdapterCount();
    header.gameDirCount = getGameDirCount();
    header.gameExeCount = getGameExecutableCount();
    header.sampCount    = getSampVersionCount();
    header.groupCount   = getGroupCount();
    header.serverCount  = getServerCount();
    header.profileCount = getProfileCount();

    file.write(reinterpret_cast<char*>(&header), sizeof(header));

    auto writeUint16   = [&file](quint16 number) { writeNumber<quint16>(file, number); };
    auto writeUint32   = [&file](quint32 number) { writeNumber<quint32>(file, number); };
    auto writeString8  = [&file](QString& str) { return writeString<quint8>(file, str); };
    auto writeString16 = [&file](QString& str) { return writeString<quint16>(file, str); };
    auto writeString32 = [&file](QString& str) { return writeString<quint32>(file, str); };

    for (quint32 i{0}; i < header.proxyCount; ++i) {
        auto pr{proxys_[i]};
        writeString8(pr.first);
        writeString8(pr.second);
    }

    for (quint32 i{0}; i < header.adapterCount; ++i) {
        writeString8(adapters_[i]);
    }

    for (quint32 i{0}; i < header.gameDirCount; ++i) {
        writeString16(gameDirs_[i]);
    }

    for (quint32 i{0}; i < header.gameExeCount; ++i) {
        writeString8(gameExecutables_[i]);
    }

    for (quint32 i{0}; i < header.sampCount; ++i) {
        auto sv{sampVersions_[i]};
        writeString8(sv.first);
        writeString8(sv.second);
    }

    for (quint32 i{0}; i < header.groupCount; ++i) {
        writeString8(groups_[i]);
    }

    for (quint32 i{0}; i < header.serverCount; ++i) {
        Server server{servers_[i]};

        writeUint32(server.group);

        QString ip;
        quint16 port;
        parseAddress(server.address, ip, port);

        writeString8(ip);
        writeUint16(port);

        writeString8(server.password);
        writeString8(server.rcon);
        writeUint32(server.profile);
    }

    for (quint32 i{0}; i < header.profileCount; ++i) {
        Profile profile{profiles_[i]};

        writeString8(profile.name);
        writeString8(profile.nickname);
        writeUint32(profile.proxy);
        writeUint32(profile.adapter);
        writeUint32(profile.gameDir);
        writeUint32(profile.gameExe);
        writeUint32(profile.samp);
        writeString32(profile.comment);
    }

    file.close();

    return true;
}

bool SettingsData::save()
{
    return save(filePath_);
}

QString SettingsData::getFilePath() const
{
    return filePath_;
}

quint32 SettingsData::getProxyCount() const
{
    return proxys_.count();
}

QPair<QString, QString> SettingsData::getProxy(quint32 id) const
{
    return proxys_[id];
}

void SettingsData::setProxy(quint32 id, const QPair<QString, QString>& proxy)
{
    proxys_[id] = proxy;
}

quint32 SettingsData::addProxy(const QPair<QString, QString>& proxy)
{
    proxys_.append(proxy);
    return proxys_.lastIndexOf(proxy);
}

void SettingsData::deleteProxy(quint32 id)
{
    for (auto& profile : profiles_) {
        if (profile.proxy == id) {
            profile.proxy = -1;
        }
    }
    proxys_.removeAt(id);
}

quint32 SettingsData::getAdapterCount() const
{
    return adapters_.count();
}

QString SettingsData::getAdapter(quint32 id) const
{
    return adapters_[id];
}

void SettingsData::setAdapter(quint32 id, const QString& adapter)
{
    adapters_[id] = adapter;
}

quint32 SettingsData::addAdapter(const QString& adapter)
{
    adapters_.append(adapter);
    return adapters_.lastIndexOf(adapter);
}

void SettingsData::deleteAdapter(quint32 id)
{
    for (auto& profile : profiles_) {
        if (profile.adapter == id) {
            profile.adapter = -1;
        }
    }
    adapters_.removeAt(id);
}

quint32 SettingsData::getGameDirCount() const
{
    return gameDirs_.count();
}

QString SettingsData::getGameDir(quint32 id) const
{
    return gameDirs_[id];
}

void SettingsData::setGameDir(quint32 id, const QString& gameDir)
{
    gameDirs_[id] = gameDir;
}

quint32 SettingsData::addGameDir(const QString& gameDir)
{
    int idx{gameDirs_.lastIndexOf(gameDir)};
    if (idx != -1) {
        return idx;
    }
    gameDirs_.append(gameDir);
    return gameDirs_.indexOf(gameDir);
}

void SettingsData::deleteGameDir(quint32 id)
{
    for (auto& profile : profiles_) {
        if (profile.gameDir == id) {
            profile.gameDir = -1;
        }
    }
    gameDirs_.removeAt(id);
}

int SettingsData::findGameDir(const QString& gameDir) const
{
    for (quint32 i{0}; i < getGameDirCount(); ++i) {
        if (getGameDir(i) == gameDir) {
            return i;
        }
    }
    return -1;
}

void SettingsData::maybeCleanUpGameDir(quint32 id)
{
    bool found{false};
    for (auto& profile : profiles_) {
        if (profile.gameDir == id) {
            found = true;
            break;
        }
    }
    if (!found) {
        deleteGameDir(id);
    }
}

quint32 SettingsData::getGameExecutableCount() const
{
    return gameExecutables_.count();
}

QString SettingsData::getGameExecutable(quint32 id) const
{
    return gameExecutables_[id];
}

void SettingsData::setGameExecutable(quint32 id, const QString& gameExe)
{
    gameExecutables_[id] = gameExe;
}

quint32 SettingsData::addGameExecutable(const QString& gameExe)
{
    int idx{gameExecutables_.indexOf(gameExe)};
    if (idx != -1) {
        return idx;
    }
    gameExecutables_.append(gameExe);
    return gameExecutables_.indexOf(gameExe);
}

void SettingsData::deleteGameExecutable(quint32 id)
{
    for (auto& profile : profiles_) {
        if (profile.gameExe == id) {
            profile.gameExe = -1;
        }
    }
    gameExecutables_.removeAt(id);
}

int SettingsData::findGameExecutable(const QString& gameExe) const
{
    for (quint32 i{0}; i < getGameExecutableCount(); ++i) {
        if (getGameExecutable(i) == gameExe) {
            return i;
        }
    }
    return -1;
}

void SettingsData::maybeCleanUpGameExecutable(quint32 id)
{
    bool found{false};
    for (auto& profile : profiles_) {
        if (profile.gameExe == id) {
            found = true;
            break;
        }
    }
    if (!found) {
        deleteGameExecutable(id);
    }
}

quint32 SettingsData::getSampVersionCount() const
{
    return sampVersions_.count();
}

QPair<QString, QString> SettingsData::getSampVersion(quint32 id) const
{
    return sampVersions_[id];
}

void SettingsData::setSampVersion(quint32 id, const QPair<QString, QString>& samp)
{
    sampVersions_[id] = samp;
}

quint32 SettingsData::addSampVersion(const QPair<QString, QString>& samp)
{
    sampVersions_.append(samp);
    return sampVersions_.indexOf(samp);
}

void SettingsData::deleteSampVersion(quint32 id)
{
    for (auto& profile : profiles_) {
        if (profile.samp == id) {
            profile.samp = -1;
        }
    }
    sampVersions_.removeAt(id);
}

int SettingsData::findSampVersionByFileName(QString& fileName) const
{
    for (int i{0}; i < getSampVersionCount(); ++i) {
        auto v{getSampVersion(i)};
        if (v.second == fileName) {
            return i;
        }
    }
    return -1;
}

quint32 SettingsData::getGroupCount() const
{
    return groups_.count();
}

QString SettingsData::getGroup(quint32 id) const
{
    return groups_[id];
}

void SettingsData::setGroup(quint32 id, const QString& group)
{
    groups_[id] = group;
}

quint32 SettingsData::addGroup(const QString& group)
{
    int idx{groups_.indexOf(group)};
    if (idx != -1) {
        return idx;
    }
    groups_.append(group);
    return groups_.indexOf(group);
}

void SettingsData::deleteGroup(quint32 id)
{
    for (int i{0}; i < getServerCount(); ++i) {
        auto server{getServer(i)};
        if (server.group == id) {
            deleteServer(i);
        }
    }
    groups_.removeAt(id);
}

quint32 SettingsData::getServerCount() const
{
    return servers_.count();
}

SettingsData::Server SettingsData::getServer(quint32 id) const
{
    return servers_[id];
}

void SettingsData::setServer(quint32 id, Server server)
{
    servers_[id] = server;
}

quint32 SettingsData::addServer(SettingsData::Server server)
{
    servers_.append(server);
    return servers_.count() - 1;
}

void SettingsData::deleteServer(quint32 id)
{
    servers_.removeAt(id);
}

int SettingsData::findServerByAddress(const QString& address, const quint32 group) const
{
    if (group == -1) {
        for (int i{0}; i < getServerCount(); ++i) {
            if (getServer(i).address == address) {
                return i;
            }
        }
    } else {
        for (int i{0}; i < getServerCount(); ++i) {
            auto srv{getServer(i)};
            if (srv.address == address && srv.group == group) {
                return i;
            }
        }
    }
    return -1;
}

quint32 SettingsData::getProfileCount() const
{
    return profiles_.count();
}

SettingsData::Profile SettingsData::getProfile(quint32 id) const
{
    return profiles_[id];
}

void SettingsData::setProfile(quint32 id, Profile profile)
{
    profiles_[id] = profile;
}

void SettingsData::addProfile(const SettingsData::Profile profile)
{
    profiles_.append(profile);
}

void SettingsData::deleteProfile(quint32 id)
{
    for (int i{0}; i < getServerCount(); ++i) {
        auto srv{getServer(i)};
        if (srv.profile == id) {
            srv.profile = 0;
            setServer(i, srv);
        }
    }

    profiles_.removeAt(id);
}
