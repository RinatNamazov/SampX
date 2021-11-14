/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SampFavorites.hpp
 *  DESCRIPTION:    Read/write file of favorite samp servers
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QList>
#include <QString>
#include <QStringView>

class SampFavorites {
public:
    typedef struct {
        char    tag[4]; // SAMP
        quint32 version;
        quint32 serversCount;
    } FileHeader;

    typedef struct {
        QString address;
        quint16 port;
        QString hostname;
        QString password;
        QString rcon;
    } Server;

    static QString getDefaultSampFilePath();

    bool load(const QString& filePath);
    bool save(const QString& filePath);
    bool save();

    QString getFilePath() const;

    quint32 getServersCount() const;

    Server getServer(quint32 id) const;
    void   setServer(quint32 id, Server server);
    void   addServer(Server server);

private:
    QString       filePath_;
    QList<Server> servers_;
};
