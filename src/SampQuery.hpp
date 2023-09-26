/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           Samp query.cpp
 *  DESCRIPTION:    Samp query mechanism
 *  COPYRIGHT:      (c) 2021, 2023 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QMap>
#include <QNetworkDatagram>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QUdpSocket;
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

#pragma pack(push, 1)
struct QueryPacket {
    char tag[4];
    char host[4];
    union {
        quint16 port;
        quint8  portParts[2];
    };
    quint8 opcode;
};
#pragma pack(pop)

enum class QueryPaketOpcode : quint8 {
    Ping               = 'p', // Ping paket.
    Info               = 'i', // Total info about server.
    Rules              = 'r', // Rules, also known as key values or params (gravity, website) etc.
    ClientList         = 'c', // Basic (NickName and score) client list.
    DetailedClientList = 'd', // Detailed (include ID and ping) client list.
    Rcon               = 'x'  // RCON packet.
};

struct ServerInfoResponse {
    bool    isClosedServer;
    quint16 curPlayers;
    quint16 maxPlayers;
    QString hostname;
    QString mode;
    QString language;
};

class SampQuery : public QObject {
    Q_OBJECT

public:
    SampQuery(const QString& address = "");
    ~SampQuery();

    QString getAddress() const;
    void    setAddress(const QString& address);
    void    setMasterServerAddress(const QString& address);

    void requestMasterServerList();

    void requestInformation();
    void requestRules();
    void requestClientList();
    void requestDetailedPlayerInfo();
    void requestPing();

signals:
    void masterServerResponded(const QStringList& servers);
    void pingCalculated(int ping);
    void serverInfoResponse(ServerInfoResponse resp);
    void serverRulesResponse(QMap<QString, QString> rules);

private slots:
    void handleHttpResponse(QNetworkReply* reply);
    void readPendingDatagrams();

private:
    QUdpSocket*            socket_;
    QNetworkAccessManager* networkManager_;

    QUrl    masterServerUrl_;
    QString address_;
    QString ip_;
    quint16 port_;

    quint64 sends_;
    quint64 recived_;
    int     sendTime_;

    void processDatagram(QByteArray&& datagram);

    void send(QByteArray data);

    QByteArray assemblePacket(const QueryPaketOpcode opcode);
};
