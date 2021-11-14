/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           utils.cpp
 *  DESCRIPTION:    Utils
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QString>

bool isValidServerAddress(const QString& address);

void parseAddress(QString address, QString& ip, quint16& port);

QString formatLastWindowsError(QString functionName);
