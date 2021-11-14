/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SampQuery.cpp
 *  DESCRIPTION:    Samp query mechanism
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#include <QDataStream>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>
#include <QTime>
#include <QUdpSocket>

#include <Windows.h>

#include "SampQuery.hpp"

#include "utils.hpp"

SampQuery::SampQuery(const QString& address)
    : QObject()
    , sends_(0)
    , recived_(0)
    , masterServerUrl_("http://lists.sa-mp.com/0.3.7/servers")
{
    if (!address.isEmpty()) {
        setAddress(address);
    }

    socket_ = new QUdpSocket(this);

    QObject::connect(socket_, &QUdpSocket::readyRead, this, &SampQuery::readPendingDatagrams);

    networkManager_ = new QNetworkAccessManager(this);
    QObject::connect(networkManager_,
                     &QNetworkAccessManager::finished,
                     this,
                     &SampQuery::handleHttpResponse);
}

SampQuery::~SampQuery()
{
    QObject::disconnect(socket_, &QUdpSocket::readyRead, this, &SampQuery::readPendingDatagrams);

    delete networkManager_;
    delete socket_;
}

QString SampQuery::getAddress() const
{
    return address_;
}

void SampQuery::setAddress(const QString& address)
{
    address_ = address;
    parseAddress(address_, ip_, port_);

    if (QHostAddress(ip_).protocol() != QAbstractSocket::IPv4Protocol) {
        QHostInfo::lookupHost(ip_, this, [this](const QHostInfo& host) {
            if (host.error() != QHostInfo::HostInfoError::NoError) {
                qDebug() << "Lookup failed:" << host.errorString();
                return;
            }
            ip_ = host.addresses().first().toString();
        });
    }
}

void SampQuery::setMasterServerAddress(const QString& address)
{
    masterServerUrl_.setUrl(address);
}

void SampQuery::requestMasterServerList()
{
    QNetworkRequest req(masterServerUrl_);
    networkManager_->get(req);
}

void SampQuery::requestHttpPing()
{
    QNetworkRequest req(QUrl("http://" + ip_));
    networkManager_->get(req);
}

void SampQuery::requestInformation()
{
    send(assemblePacket(QueryPaketOpcode::Info));
}

void SampQuery::requestRules()
{
    send(assemblePacket(QueryPaketOpcode::Rules));
}

void SampQuery::requestClientList()
{
    send(assemblePacket(QueryPaketOpcode::ClientList));
}

void SampQuery::requestDetailedPlayerInfo()
{
    send(assemblePacket(QueryPaketOpcode::DetailedClientList));
}

void SampQuery::requestPing()
{
    QByteArray data{assemblePacket(QueryPaketOpcode::Ping)};

    quint32 tickCount{GetTickCount()};
    QByteArray tickCountData(sizeof(quint32), Qt::Uninitialized);
    *reinterpret_cast<quint32*>(tickCountData.data()) = tickCount;

    data += tickCountData;

    send(data);
}

void SampQuery::handleHttpResponse(QNetworkReply* reply)
{
    if (reply->error()) {
        return;
    }

    if (reply->request().url() == masterServerUrl_) {
        QString answer{reply->readAll()};
        auto servers{answer.split("\n")};
        servers.pop_back(); // Deleting the last transition to a new line.

        emit masterServerResponded(servers);
    }
}

void SampQuery::readPendingDatagrams()
{
    while (socket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram{socket_->receiveDatagram()};

        if (datagram.senderAddress().toIPv4Address() == QHostAddress(ip_).toIPv4Address()) {
            if (recived_ > sends_) {
                return;
            }
            ++recived_;

            processDatagram(std::move(datagram.data()));
        }
    }
}

template<typename T>
QString readString(QDataStream& data, QTextCodec* codec)
{
    T stringLength;
    data >> stringLength;
    if (stringLength > 0) {
        QByteArray stringData(stringLength, Qt::Uninitialized);
        data.readRawData(stringData.data(), stringLength);
        return codec->toUnicode(stringData);
    }
    return "-";
};

void SampQuery::processDatagram(QByteArray&& datagram)
{
    emit pingCalculated(QTime::currentTime().msecsSinceStartOfDay() - sendTime_);

    if (datagram.size() < sizeof(QueryPacket)) {
        return;
    }

    QueryPacket* packet{reinterpret_cast<QueryPacket*>(datagram.data())};
    //QByteArray data{datagram.mid(sizeof(QueryPacket))};
    QDataStream data(datagram.mid(sizeof(QueryPacket)));
    data.setByteOrder(QDataStream::LittleEndian);

    QTextCodec* codec{QTextCodec::codecForName("cp1251")};

    switch (static_cast<QueryPaketOpcode>(packet->opcode)) {
        case QueryPaketOpcode::Info: {
            ServerInfoResponse resp;

            data >> resp.isClosedServer;
            data >> resp.curPlayers;
            data >> resp.maxPlayers;

            resp.hostname = readString<quint32>(data, codec);
            resp.mode     = readString<quint32>(data, codec);
            resp.language = readString<quint32>(data, codec);

            emit serverInfoResponse(resp);
            break;
        }
        case QueryPaketOpcode::Rules: {
            quint16 ruleCount;
            data >> ruleCount;

            QMap<QString, QString> rules;

            for (quint16 i{0}; i < ruleCount; ++i) {
                QString key{readString<quint8>(data, codec)};
                QString value{readString<quint8>(data, codec)};

                rules.insert(key, value);
            }

            emit serverRulesResponse(rules);
            break;
        }
        default:
            break;
    }
}

void SampQuery::send(QByteArray data)
{
    socket_->writeDatagram(data, QHostAddress(ip_), port_);
    sendTime_ = QTime::currentTime().msecsSinceStartOfDay();
    ++sends_;
}

QByteArray SampQuery::assemblePacket(const QueryPaketOpcode opcode)
{
    QByteArray data(sizeof(QueryPacket), Qt::Uninitialized);

    QueryPacket* packet{reinterpret_cast<QueryPacket*>(data.data())};

    packet->tag[0] = 'S';
    packet->tag[1] = 'A';
    packet->tag[2] = 'M';
    packet->tag[3] = 'P';

    // Split the IP into 4 bytes.
    auto splitedIp{ip_.split('.')};
    if (splitedIp.count() == 4) {
        for (int i{0}; i < 4; ++i) {
            packet->host[i] = static_cast<char>(splitedIp[i].toInt());
        }
    }

    // Split the port into 2 bytes.
    packet->portParts[0] = static_cast<quint8>(port_ & 0xFF);
    packet->portParts[1] = static_cast<quint8>(port_ >> 8 & 0xFF);

    packet->opcode = static_cast<quint8>(opcode);

    return data;
}
