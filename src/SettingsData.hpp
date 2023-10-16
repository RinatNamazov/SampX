/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SettingsData.hpp
 *  DESCRIPTION:    Read/write config file
 *  COPYRIGHT:      (c) 2021, 2023 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QList>
#include <QPair>
#include <QString>

class SettingsData {
public:
#pragma pack(push, 1)
    typedef struct {
        char    tag[14]; // RINWARES_SAMPX
        quint32 version;
        quint32 proxyCount;
        quint32 adapterCount;
        quint32 gameDirCount;
        quint32 gameExeCount;
        quint32 sampCount;
        quint32 groupCount;
        quint32 serverCount;
        quint32 profileCount;
    } FileHeader;
#pragma pack(pop)

    struct Server {
        quint32 group;
        QString address;
        QString password;
        QString rcon;
        quint32 profile;
    };

    struct Profile {
        QString name;
        QString nickname;
        quint32 proxy;
        quint32 adapter;
        quint32 gameDir;
        quint32 gameExe;
        quint32 samp;
        QString comment;

        Profile()
            : proxy(-1)
            , adapter(-1)
            , gameDir(-1)
            , gameExe(-1)
            , samp(-1)
        {}
    };

    bool load(const QString& filePath);
    bool save(const QString& filePath);
    bool save();

    QString getFilePath() const;

    quint32                 getProxyCount() const;
    QPair<QString, QString> getProxy(quint32 id) const;
    void                    setProxy(quint32 id, const QPair<QString, QString>& proxy);
    quint32                 addProxy(const QPair<QString, QString>& proxy);
    void                    deleteProxy(quint32 id);

    quint32 getAdapterCount() const;
    QString getAdapter(quint32 id) const;
    void    setAdapter(quint32 id, const QString& adapter);
    quint32 addAdapter(const QString& adapter);
    void    deleteAdapter(quint32 id);

    quint32 getGameDirCount() const;
    QString getGameDir(quint32 id) const;
    void    setGameDir(quint32 id, const QString& gameDir);
    quint32 addGameDir(const QString& gameDir);
    void    deleteGameDir(quint32 id);
    int     findGameDir(const QString& gameDir) const;
    void    maybeCleanUpGameDir(quint32 id);

    quint32 getGameExecutableCount() const;
    QString getGameExecutable(quint32 id) const;
    void    setGameExecutable(quint32 id, const QString& gameExe);
    quint32 addGameExecutable(const QString& gameExe);
    void    deleteGameExecutable(quint32 id);
    int     findGameExecutable(const QString& gameExe) const;
    void    maybeCleanUpGameExecutable(quint32 id);

    quint32                 getSampVersionCount() const;
    QPair<QString, QString> getSampVersion(quint32 id) const;
    void                    setSampVersion(quint32 id, const QPair<QString, QString>& samp);
    quint32                 addSampVersion(const QPair<QString, QString>& samp);
    void                    deleteSampVersion(quint32 id);
    int                     findSampVersionByFileName(QString& fileName) const;

    quint32 getGroupCount() const;
    QString getGroup(quint32 id) const;
    void    setGroup(quint32 id, const QString& group);
    quint32 addGroup(const QString& group);
    void    deleteGroup(quint32 id);

    quint32 getServerCount() const;
    Server  getServer(quint32 id) const;
    void    setServer(quint32 id, Server server);
    quint32 addServer(Server server);
    void    deleteServer(quint32 id);
    int     findServerByAddress(const QString& address, const quint32 group = -1) const;
    void    moveServer(quint32 from, quint32 to);

    quint32 getProfileCount() const;
    Profile getProfile(quint32 id) const;
    void    setProfile(quint32 id, Profile profile);
    quint32 addProfile(Profile profile);
    void    deleteProfile(quint32 id);

private:
    QString filePath_;

    QList<QString>                 groups_;
    QList<Server>                  servers_;
    QList<Profile>                 profiles_;
    QList<QPair<QString, QString>> proxys_;
    QList<QString>                 adapters_;
    QList<QString>                 gameDirs_;
    QList<QString>                 gameExecutables_;
    QList<QPair<QString, QString>> sampVersions_;
};
