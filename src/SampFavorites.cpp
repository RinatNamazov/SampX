/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SampFavorites.cpp
 *  DESCRIPTION:    Read/write file of favorite samp servers
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#include <QFile>
#include <QStandardPaths>
#include <QTextCodec>

#include "SampFavorites.hpp"

QString SampFavorites::getDefaultSampFilePath()
{
    return QStandardPaths::locate(QStandardPaths::DocumentsLocation,
                                  "GTA San Andreas User Files/SAMP/USERDATA.DAT");
}

bool SampFavorites::load(const QString& filePath)
{
    filePath_ = filePath;

    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    FileHeader header;

    file.read(reinterpret_cast<char*>(&header.tag), sizeof(header.tag));
    file.read(reinterpret_cast<char*>(&header.version), sizeof(header.version));
    file.read(reinterpret_cast<char*>(&header.serversCount), sizeof(header.serversCount));

    QTextCodec* codec{QTextCodec::codecForName("cp1251")};

    for (quint32 i{0}; i < header.serversCount; ++i) {
        Server  server;
        quint32 length;

        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        server.address = codec->toUnicode(file.read(length));

        file.read(reinterpret_cast<char*>(&server.port), sizeof(quint32));

        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        server.hostname = codec->toUnicode(file.read(length));

        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        server.password = codec->toUnicode(file.read(length));

        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        server.rcon = codec->toUnicode(file.read(length));

        addServer(server);
    }

    file.close();

    return true;
}

bool SampFavorites::save(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    FileHeader header;
    header.tag[0]       = 'S';
    header.tag[1]       = 'A';
    header.tag[2]       = 'M';
    header.tag[3]       = 'P';
    header.version      = 1;
    header.serversCount = getServersCount();

    file.write(header.tag, sizeof(header.tag));
    file.write(reinterpret_cast<char*>(&header.version), sizeof(header.version));
    file.write(reinterpret_cast<char*>(&header.serversCount), sizeof(header.serversCount));

    QTextCodec* codec{QTextCodec::codecForName("cp1251")};

    for (quint32 i{0}; i < header.serversCount; ++i) {
        Server server{getServer(i)};
        quint32 length;

        length = server.address.length();
        file.write(reinterpret_cast<char*>(&length), sizeof(length));
        file.write(codec->fromUnicode(server.address).toStdString().c_str(), length);

        file.write(reinterpret_cast<char*>(&server.port), sizeof(quint32));

        length = server.hostname.length();
        file.write(reinterpret_cast<char*>(&length), sizeof(length));
        file.write(codec->fromUnicode(server.hostname).toStdString().c_str(), length);

        length = server.password.length();
        file.write(reinterpret_cast<char*>(&length), sizeof(length));
        file.write(codec->fromUnicode(server.password).toStdString().c_str(), length);

        length = server.rcon.length();
        file.write(reinterpret_cast<char*>(&length), sizeof(length));
        file.write(codec->fromUnicode(server.rcon).toStdString().c_str(), length);
    }

    file.close();

    return true;
}

bool SampFavorites::save()
{
    return save(filePath_);
}

QString SampFavorites::getFilePath() const
{
    return filePath_;
}

quint32 SampFavorites::getServersCount() const
{
    return servers_.size();
}

SampFavorites::Server SampFavorites::getServer(quint32 id) const
{
    return servers_[id];
}

void SampFavorites::setServer(quint32 id, SampFavorites::Server server)
{
    servers_[id] = server;
}

void SampFavorites::addServer(const SampFavorites::Server server)
{
    servers_.append(server);
}
