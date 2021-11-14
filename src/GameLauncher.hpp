/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           GameLauncher.cpp
 *  DESCRIPTION:    Game launcher
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QDir>
#include <QPair>
#include <QString>

#include <Windows.h>

class GameLauncher : public QObject {
    Q_OBJECT

public:
    GameLauncher();

    void setGameDir(QDir gameDir);
    void setGameExe(QString gameExe);
    void setSampDll(QString sampDll);
    void setNickName(QString nickname);
    void setAddress(QString address);
    void setPassword(QString password);

    void setNetworkAdapter(QString address);
    void setProxy(QPair<QString, QString> proxy);

    bool launch();

    void replaceSampLibrary(const QString& sampDll);

private:
    QDir    gameDir_;
    QString gameExe_;
    QString sampDll_;

    QString nickname_;
    QString address_;
    QString ip_;
    quint16 port_;
    QString password_;

    QString                 networkAdapter_;
    QPair<QString, QString> proxy_;

    LPTHREAD_START_ROUTINE loadLibraryFunctionAddress_;
    PROCESS_INFORMATION    processInfo_;

    bool injectLibrary(QString libraryPath);
};
