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

#include <Windows.h>
#include <strsafe.h>

#include "utils.hpp"

bool isValidServerAddress(const QString& address)
{
    if (address.isEmpty()) {
        return false;
    }
    int pos{address.lastIndexOf(":")};
    if (pos == -1) {
        return false;
    }
    bool ok;
    int port{address.midRef(pos + 1).toInt(&ok, 10)};
    if (!ok) {
        return false;
    }
    if (port < 0 || port > 65535) {
        return false;
    }
    return true;
}

void parseAddress(QString address, QString& ip, quint16& port)
{
    int pos{address.lastIndexOf(':')};
    if (pos != -1) {
        ip   = address.mid(0, pos);
        port = address.mid(pos + 1).toUShort();
    }
}

QString formatLastWindowsError(QString functionName)
{
    DWORD lastError{GetLastError()};

    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                      | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  lastError,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  reinterpret_cast<LPTSTR>(&lpMsgBuf),
                  0,
                  NULL);

    QString formatedError = QString("%1 failed with error %2: %3")
                                .arg(functionName)
                                .arg(lastError)
                                .arg(reinterpret_cast<LPTSTR>(lpMsgBuf));

    LocalFree(lpMsgBuf);

    return formatedError;
}
