/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           MainWindow.hpp
 *  DESCRIPTION:    Main window
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <QTranslator>

#include "GameLauncher.hpp"
#include "SampQuery.hpp"
#include "SettingsData.hpp"

QT_BEGIN_NAMESPACE
class QSettings;
class QTableWidgetItem;
class QTimer;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class CustomSortFilterProxyModel : public QSortFilterProxyModel {
public:
    CustomSortFilterProxyModel(QObject* parent = nullptr);

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};

class SampVersionsDialog;
class AdaptersDialog;
class ProxysDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void changeEvent(QEvent* event);
    void closeEvent(QCloseEvent*);

private slots:
    void masterServerResponded(const QStringList& servers);

    void languageChanged(QAction* action);

    void on_actionCheckForUpdates_triggered();
    void on_actionSite_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

    void on_actionSamp_triggered();
    void on_actionProxy_triggered();
    void on_actionAdapter_triggered();

    void on_actionDark_triggered();
    void on_actionLight_triggered();

    void on_connectButton_clicked();

    void on_search_textChanged(const QString& text);

    void on_group_currentIndexChanged(int index);
    void on_group_editTextChanged(const QString& text);

    void on_addGroupButton_clicked();
    void on_deleteGroupButton_clicked();
    void on_addServerButton_clicked();

    void on_servers_doubleClicked(const QModelIndex& index);
    void on_servers_sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void on_servers_currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_servers_customContextMenuRequested(const QPoint& pos);

    void on_profile_currentIndexChanged(int index);

    void on_profile_editTextChanged(const QString& text);
    void on_nickname_editingFinished();
    void on_adapter_currentIndexChanged(int index);
    void on_proxy_currentIndexChanged(int index);
    void on_gameDir_editingFinished();
    void on_selectGameDirButton_clicked();
    void on_gameExe_editingFinished();
    void on_sampVersion_currentIndexChanged(int index);
    void on_comment_textChanged();

    void on_createNewProfileButton_clicked();
    void on_deleteProfileButton_clicked();

private:
    Ui::MainWindow*             ui_;
    QStandardItemModel*         serversModel_;
    CustomSortFilterProxyModel* serversProxyModel_;
    SampVersionsDialog*         sampVersionsDialog_;
    AdaptersDialog*             adaptersDialog_;
    ProxysDialog*               proxysDialog_;
    QSettings*                  settings_;
    QTranslator                 translator_;
    QTranslator                 translatorQt_;
    QString                     currLang_;
    QString                     langPath_;
    GameLauncher                gameLauncher_;
    SampQuery                   sampQuery_;
    SettingsData                config_;
    QTimer*                     pingTimer_;
    bool                        isManualProfileChanging_;

    int            getCurrentRow();
    QStandardItem* getItemFromTable(int row, int column);

    void loadLastTheme();
    void setLastWindowsSizeAndPos();
    void loadLastLanguage();
    void setLastServersColumnWidth();

    void createDefaultConfig();

    void loadTheme(const QString& theme);

    void createLanguageMenu();
    void switchTranslator(QTranslator& translator, const QString& filePath);
    void loadLanguage(const QString& newLanguage);

    void updateServerInfo();
    void pingServerInTable(int row);

    void launchGameWithServerOnRow(int row);
    void launchGame(const QString& address, const QString& password);
};
