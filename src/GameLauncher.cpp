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

#include <QMessageBox>

#include <Windows.h>

#include "GameLauncher.hpp"

#include "utils.hpp"

GameLauncher::GameLauncher()
    : processInfo_({0})
    , gameExe_("gta_sa.exe")
    , sampDll_("samp.dll")
{
    HMODULE kernel32Library{GetModuleHandleA("kernel32.dll")};
    if (!kernel32Library) {
        QMessageBox::critical(nullptr,
                              tr("Error"),
                              formatLastWindowsError("GetModuleHandleA(\"kernel32.dll\")"));
        return;
    }

    loadLibraryFunctionAddress_ = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        GetProcAddress(kernel32Library, "LoadLibraryA"));
    if (!loadLibraryFunctionAddress_) {
        QMessageBox::critical(nullptr,
                              tr("Error"),
                              formatLastWindowsError("GetProcAddress(\"LoadLibraryA\")"));
        return;
    }
}

void GameLauncher::setGameDir(QDir gameDir)
{
    gameDir_ = gameDir;
}

void GameLauncher::setGameExe(QString gameExe)
{
    gameExe_ = gameExe;
}

void GameLauncher::setSampDll(QString sampDll)
{
    sampDll_ = sampDll;
}

void GameLauncher::setNickName(QString nickname)
{
    nickname_ = nickname;
}

void GameLauncher::setAddress(QString address)
{
    address_ = address;

    int pos{address_.lastIndexOf(':')};
    if (pos != -1) {
        ip_   = address_.mid(0, pos);
        port_ = address_.mid(pos + 1).toShort();
    }
}

void GameLauncher::setPassword(QString password)
{
    password_ = password;
}

void GameLauncher::setNetworkAdapter(QString address)
{
    networkAdapter_ = address;
}

void GameLauncher::setProxy(QPair<QString, QString> proxy)
{
    proxy_ = proxy;
}

bool GameLauncher::launch()
{
    QString commandLine{QString("-c -h %1 -p %2 -n %3").arg(ip_).arg(port_).arg(nickname_)};
    if (!password_.isEmpty()) {
        commandLine += QString(" -z %1").arg(password_);
    }
    if (!networkAdapter_.isEmpty()) {
        commandLine += " --adapter_address " + networkAdapter_;
    }
    if (!proxy_.first.isEmpty()) {
        commandLine += " --proxy_address " + proxy_.first;
        if (!proxy_.second.isEmpty()) {
            commandLine += " --proxy_key " + proxy_.second;
        }
    }

    STARTUPINFO startupInfo{{0}};

    if (!CreateProcessA(gameDir_.filePath(gameExe_).toLocal8Bit().data(),
                        commandLine.toLocal8Bit().data(),
                        0,
                        0,
                        FALSE,
                        DETACHED_PROCESS | CREATE_SUSPENDED,
                        0,
                        gameDir_.path().toLocal8Bit().data(),
                        &startupInfo,
                        &processInfo_)) {
        QMessageBox::critical(nullptr,
                              tr("Failed to start the game"),
                              formatLastWindowsError("CreateProcessA"));
        return false;
    }

    if (!injectLibrary(gameDir_.filePath(sampDll_))) {
        TerminateProcess(processInfo_.hProcess, 0);
        return false;
    }

    if (ResumeThread(processInfo_.hThread) == -1) {
        TerminateProcess(processInfo_.hProcess, 0);
        QMessageBox::critical(nullptr,
                              tr("Failed to start the game"),
                              formatLastWindowsError("ResumeThread"));
        return false;
    }

    CloseHandle(processInfo_.hProcess);
    CloseHandle(processInfo_.hThread);

    return true;
}

bool GameLauncher::injectLibrary(QString libraryPath)
{
    try {
        LPVOID parameter = reinterpret_cast<LPVOID>(VirtualAllocEx(processInfo_.hProcess,
                                                                   0,
                                                                   libraryPath.length(),
                                                                   MEM_RESERVE | MEM_COMMIT,
                                                                   PAGE_READWRITE));
        if (!parameter) {
            throw std::exception(formatLastWindowsError("VirtualAllocEx").toStdString().c_str());
        }

        if (!WriteProcessMemory(processInfo_.hProcess,
                                parameter,
                                libraryPath.toLocal8Bit().data(),
                                libraryPath.length(),
                                0)) {
            throw std::exception(formatLastWindowsError("WriteProcessMemory").toStdString().c_str());
        }

        HANDLE loadLibraryThread = CreateRemoteThread(processInfo_.hProcess,
                                                      0,
                                                      0,
                                                      loadLibraryFunctionAddress_,
                                                      parameter,
                                                      0,
                                                      0);
        if (!loadLibraryThread) {
            throw std::exception(formatLastWindowsError("CreateRemoteThread").toStdString().c_str());
        }

        if (WaitForSingleObject(loadLibraryThread, INFINITE) == WAIT_FAILED) {
            throw std::exception(
                formatLastWindowsError("WaitForSingleObject").toStdString().c_str());
        }

        if (!VirtualFreeEx(processInfo_.hProcess, parameter, 0, MEM_RELEASE)) {
            throw std::exception(formatLastWindowsError("VirtualFreeEx").toStdString().c_str());
        }

        CloseHandle(loadLibraryThread);
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, tr("Failed to inject the library"), e.what());
        return false;
    }

    return true;
}

void GameLauncher::replaceSampLibrary(const QString& sampDll)
{
    if (sampDll == sampDll_) {
        return;
    }

    // Todo: Maybe create hardlink/symlink?

    QString sampDllPath{QFileInfo(gameDir_, sampDll_).filePath()};
    if (QFile::exists(sampDllPath)) {
        // If the game is already running, you will not be able to delete the library.
        QString trashDir{gameDir_.absoluteFilePath("sampx_trash")};

        QDir qd(trashDir);
        qd.removeRecursively();
        qd.mkdir(trashDir);

        QString trashDllPath{QFileInfo(trashDir, QString("samp_%1.dll").arg(qd.count())).filePath()};
        QFile::rename(sampDllPath, trashDllPath);
    }

    QString newSampDllPath{QFileInfo(gameDir_, sampDll).filePath()};

    QFile::copy(newSampDllPath, sampDllPath);
}
