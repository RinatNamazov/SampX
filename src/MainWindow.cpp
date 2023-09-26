/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           MainWindow.cpp
 *  DESCRIPTION:    Main window
 *  COPYRIGHT:      (c) 2021, 2023 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QList>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVector>
#include <QWindow>

#include <memory>

#include "MainWindow.hpp"

#include "AdaptersDialog.hpp"
#include "ProxysDialog.hpp"
#include "SampFavorites.hpp"
#include "SampQuery.hpp"
#include "SampVersions.hpp"
#include "SampVersionsDialog.hpp"
#include "ui_MainWindow.h"
#include "utils.hpp"

const QColor NotRespondingServersColor(128, 128, 128);
const QColor ClosedServersColor(168, 29, 62);

const int INDEX_INTERNET_GROUP{0};
const int INDEX_DEFAULT_GROUP{1};
const int DEFAULT_PROFILE_ID{0};

// Index of "Internet", "Default", "No" == 0
int getIndexOfGroupOrProxyOrAdapter(quint32 id)
{
    return id + 1;
}
quint32 getGroupOrProxyOrAdapterByIndex(int index)
{
    return index - 1;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("RINWARES");
    QCoreApplication::setOrganizationDomain("rinwares.com");
    QCoreApplication::setApplicationName("SampX");
    QCoreApplication::setApplicationVersion("v.1.0.2-alpha");

    ui_->setupUi(this);

    settings_ = new QSettings();

    setLastWindowsSizeAndPos();
    loadLastTheme();
    createLanguageMenu();
    loadLastLanguage();
    setLastServersColumnWidth();

    // Is this the first launch?
    if (!config_.load("./sampx.dat")) {
        createDefaultConfig();
    }

    QObject::connect(&sampQuery_,
                     &SampQuery::masterServerResponded,
                     this,
                     &MainWindow::masterServerResponded);

    pingTimer_ = new QTimer(this);
    QObject::connect(pingTimer_, &QTimer::timeout, this, &MainWindow::updateServerInfo);
    pingTimer_->start(1000);

    ui_->group->addItem("Internet");
    for (quint32 i{0}; i < config_.getGroupCount(); ++i) {
        QString group{config_.getGroup(i)};
        ui_->group->addItem(group);
    }

    for (quint32 i{0}; i < config_.getSampVersionCount(); ++i) {
        QPair<QString, QString> sv{config_.getSampVersion(i)};
        ui_->sampVersion->addItem(sv.first);
        ui_->sampVersion->setItemData(i, sv.second);
    }

    for (quint32 i{0}; i < config_.getAdapterCount(); ++i) {
        ui_->adapter->addItem(config_.getAdapter(i));
    }

    sampVersionsDialog_ = new SampVersionsDialog(&config_, ui_->sampVersion, this);
    adaptersDialog_     = new AdaptersDialog(&config_, ui_->adapter, this);
    proxysDialog_       = new ProxysDialog(&config_, ui_->proxy, this);

    for (quint32 i{0}; i < config_.getProfileCount(); ++i) {
        auto profile{config_.getProfile(i)};
        ui_->profile->addItem(profile.name);
    }

    quint32  currentProfile{0};
    QVariant lastProfile{settings_->value("profile")};
    if (lastProfile.isNull()) {
        settings_->setValue("profile", currentProfile);
    } else {
        currentProfile = lastProfile.toUInt();
    }
    if (currentProfile >= ui_->profile->count()) {
        currentProfile = 0;
    }
    ui_->profile->setCurrentIndex(currentProfile);

    ui_->servers->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui_->servers,
                     &QTableWidget::customContextMenuRequested,
                     this,
                     &MainWindow::on_customContextMenuRequested);

    int  currentGroupIndex{0};
    auto lastGroup{settings_->value("group")};
    if (lastGroup.isNull()) {
        settings_->setValue("group", currentGroupIndex);
    } else {
        currentGroupIndex = lastGroup.toInt();
    }
    if (currentGroupIndex >= ui_->group->count()) {
        currentGroupIndex = 0;
    }
    ui_->group->setCurrentIndex(currentGroupIndex);
}

void MainWindow::on_customContextMenuRequested(const QPoint& pos)
{
    int curServerIndex{ui_->servers->currentRow()};
    if (curServerIndex == -1) {
        return;
    }

    QMenu    menu(this);
    QAction* deleteAction{nullptr};
    QAction* editPasswordAction{nullptr};

    int     curGroupIndex{ui_->group->currentIndex()};
    quint32 curGroupId{getGroupOrProxyOrAdapterByIndex(curGroupIndex)};

    quint32 curServerId{ui_->servers->item(curServerIndex, 0)->data(Qt::UserRole).value<quint32>()};
    auto    curServer{config_.getServer(curServerId)};
    QString curServerAddress{ui_->servers->item(curServerIndex, 5)->text()};

    QVector<quint32> groups;

    for (quint32 groupId{0}; groupId < config_.getGroupCount(); ++groupId) {
        // Check that there is no current server in the other groups.
        if (groupId != curGroupId && config_.findServerByAddress(curServerAddress, groupId) == -1) {
            groups.append(groupId);
        }
    }

    if (!groups.isEmpty()) {
        QMenu* addToMenu{menu.addMenu(tr("Add to"))};

        for (quint32 groupId : groups) {
            QAction* act{addToMenu->addAction(config_.getGroup(groupId))};
            act->setData(groupId);
        }
    }

    if (curGroupIndex != INDEX_INTERNET_GROUP) {
        deleteAction       = menu.addAction(tr("Delete"));
        editPasswordAction = menu.addAction(tr("Edit password"));
    }

    QAction* action{menu.exec(ui_->servers->viewport()->mapToGlobal(pos))};
    if (action == nullptr) {
        return;
    }

    if (deleteAction != nullptr && action == deleteAction) { // Delete
        config_.deleteServer(curServerId);

        for (int i = curServerIndex; i < ui_->servers->rowCount(); ++i) {
            auto    hostnameItem{ui_->servers->item(i, 0)};
            quint32 serverId{hostnameItem->data(Qt::UserRole).value<quint32>()};
            hostnameItem->setData(Qt::UserRole, QVariant::fromValue(serverId - 1));
        }

        int curGroupIndex{ui_->group->currentIndex()};
        if (curGroupIndex == INDEX_INTERNET_GROUP) {
            return;
        }
        int curServerIndex{ui_->servers->currentRow()};
        if (curServerIndex == -1) {
            return;
        }

        QTableWidgetItem* addressRow{ui_->servers->item(curServerIndex, 5)};
        QVariant          userData{addressRow->data(Qt::UserRole)};
        if (!userData.isNull()) {
            auto sq{userData.value<SampQuery*>()};
            if (sq != nullptr) {
                delete sq;
            }
        }

        ui_->servers->removeRow(curServerIndex);
    } else if (editPasswordAction != nullptr && action == editPasswordAction) { // Edit password
        auto srv{config_.getServer(curServerId)};

        bool    ok;
        QString pass = QInputDialog::getText(this,
                                             tr("Add server"),
                                             tr("Enter server password"),
                                             QLineEdit::Normal,
                                             srv.password,
                                             &ok);
        if (!ok) {
            return;
        }

        srv.password = pass;
        config_.setServer(curServerId, srv);
    } else {
        // Add to
        int groupId{action->data().toInt()};

        SettingsData::Server srv;

        srv.profile = ui_->profile->currentIndex();
        srv.group   = groupId;
        srv.address = curServerAddress;

        if (deleteAction != nullptr) {
            srv.password = curServer.password;
        }

        config_.addServer(srv);
    }
}

MainWindow::~MainWindow()
{
    delete sampVersionsDialog_;
    delete adaptersDialog_;
    delete proxysDialog_;
    delete pingTimer_;
    delete ui_;
    delete settings_;
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event != nullptr) {
        switch (event->type()) {
            case QEvent::LanguageChange:
                ui_->retranslateUi(this);
                break;
            case QEvent::LocaleChange:
                loadLanguage(QLocale::system().name());
                break;
            case QEvent::ActivationChange:
                if (isActiveWindow()) {
                    pingTimer_->start();
                } else {
                    pingTimer_->stop();
                }
                break;
            default:
                break;
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent*)
{
    // Save the current position and size of the window.
    settings_->setValue("size", size());
    settings_->setValue("pos", pos());
    settings_->setValue("maximized", isMaximized());

    settings_->setValue("language", currLang_);

    settings_->setValue("group", ui_->group->currentIndex());
    settings_->setValue("profile", ui_->profile->currentIndex());

    QStringList colWidth;
    colWidth.reserve(6);
    colWidth.append(QString::number(ui_->servers->columnWidth(0))); // HostName
    colWidth.append(QString::number(ui_->servers->columnWidth(1))); // Players
    colWidth.append(QString::number(ui_->servers->columnWidth(2))); // Ping
    colWidth.append(QString::number(ui_->servers->columnWidth(3))); // Mode
    colWidth.append(QString::number(ui_->servers->columnWidth(4))); // Language
    colWidth.append(QString::number(ui_->servers->columnWidth(5))); // Address
    settings_->setValue("column_width", colWidth);

    config_.save();
}

void MainWindow::masterServerResponded(const QStringList& servers)
{
    if (ui_->group->currentIndex() == INDEX_INTERNET_GROUP) {
        ui_->servers->setRowCount(servers.count());

        for (int i{0}; i < servers.count(); i++) {
            QTableWidgetItem* hostnameItem{new QTableWidgetItem(tr("Not Available"))};
            //hostnameItem->setForeground(QBrush{NotRespondingServersColor});

            ui_->servers->setItem(i, 0, hostnameItem);
            ui_->servers->setItem(i, 1, new QTableWidgetItem());
            ui_->servers->setItem(i, 2, new QTableWidgetItem());
            ui_->servers->setItem(i, 3, new QTableWidgetItem());
            ui_->servers->setItem(i, 4, new QTableWidgetItem());
            ui_->servers->setItem(i, 5, new QTableWidgetItem(servers[i]));

            pingServerInTable(i);
        }
    }
}

void MainWindow::loadLastTheme()
{
    QString lastTheme{"dark"}; // Default theme.

    QVariant lt{settings_->value("theme")};
    if (!lt.isNull() && lt.userType() == QMetaType::QString) {
        QString theme{lt.toString()};
        if (theme == "dark" || theme == "light") {
            lastTheme = theme;
        }
    }

    loadTheme(lastTheme);
}

void MainWindow::setLastWindowsSizeAndPos()
{
    if (settings_->value("maximized", false).toBool()) {
        showMaximized();
        return;
    }

    const QSize defaultWinSize{1200, 650};

    QVariant lastSize{settings_->value("size")};
    if (lastSize.isNull() || lastSize.userType() != QMetaType::QSize) {
        resize(defaultWinSize);
    } else {
        resize(lastSize.toSize());
    }

    QList<QScreen*> screens{QGuiApplication::screens()};
    QRect           screenGeo{screens.first()->availableGeometry()};

    QPoint   lastPos;
    QVariant lp{settings_->value("pos")};
    if (lp.isNull() || lp.userType() != QMetaType::QPoint) {
        lastPos = screenGeo.center() - rect().center();
    } else {
        lastPos = lp.toPoint();

        // Is the window off the screen?
        if (!screenGeo.contains(lastPos)) {
            resize(defaultWinSize);
            lastPos = screenGeo.center() - rect().center();
        }
    }
    move(lastPos);
}

void MainWindow::loadLastLanguage()
{
    QVariant lastLanguage{settings_->value("language")};
    if (!lastLanguage.isNull() && lastLanguage.userType() == QMetaType::QString) {
        loadLanguage(lastLanguage.toString());
    }
}

void MainWindow::setLastServersColumnWidth()
{
    QVariant lastColumnWidth{settings_->value("column_width")};
    if (!lastColumnWidth.isNull() && lastColumnWidth.userType() == QMetaType::QStringList) {
        QStringList colWidth{lastColumnWidth.toStringList()};
        if (colWidth.count() <= ui_->servers->columnCount()) {
            for (int i{0}; i < colWidth.count(); ++i) {
                uint width{colWidth[i].toUInt()};
                ui_->servers->setColumnWidth(i, width);
            }
            return;
        }
    }

    // Default
    ui_->servers->setColumnWidth(0, 271); // HostName
    ui_->servers->setColumnWidth(1, 88);  // Players
    ui_->servers->setColumnWidth(2, 51);  // Ping
    ui_->servers->setColumnWidth(3, 119); // Mode
    ui_->servers->setColumnWidth(4, 114); // Language
    ui_->servers->setColumnWidth(5, 150); // Address
}

void MainWindow::createDefaultConfig()
{
    QSettings sampSettings{"HKEY_CURRENT_USER\\SOFTWARE\\SAMP", QSettings::NativeFormat};

    quint32 gameDirId = -1, gameExeId = -1;
    QString gameExePath{sampSettings.value("gta_sa_exe").toString()};
    if (!gameExePath.isEmpty()) {
        QFileInfo gtaPath(gameExePath);
        QString   gtaDir{gtaPath.dir().path()};
        QString   gtaExe{gtaPath.fileName()};

        gameDirId = config_.addGameDir(gtaDir);
        gameExeId = config_.addGameExecutable(gtaExe);
    }

    // Trying to detect several versions of samp.dll
    int          sampId{-1};
    QDirIterator it(config_.getGameDir(0), {"*.dll"}, QDir::Files);
    while (it.hasNext()) {
        QString filePath{it.next()};

        SampVersion sampVersion{SampVersions::getVersionOfSampDll(filePath)};
        if (sampVersion != SampVersion::SAMP_UNKNOWN) {
            quint32 sampVersionId{
                config_.addSampVersion({SampVersions::getSampVersionAsString(sampVersion, false),
                                        QFileInfo(filePath).fileName()})};
            if (sampId == -1) {
                sampId = sampVersionId;
            }
        }
    }

    QString nickname{sampSettings.value("PlayerName").toString()};

    quint32 favoritesId{config_.addGroup("Favorites")};

    SettingsData::Profile profile;
    profile.name     = "Default";
    profile.gameDir  = gameDirId;
    profile.gameExe  = gameExeId;
    profile.nickname = nickname;
    profile.samp     = sampId;
    config_.addProfile(profile);

    // Try importing servers from the official client.
    SampFavorites sf;
    if (sf.load(SampFavorites::getDefaultSampFilePath())) {
        for (quint32 i{0}; i < sf.getServersCount(); ++i) {
            SampFavorites::Server server = sf.getServer(i);

            SettingsData::Server srv;
            srv.group    = favoritesId;
            srv.address  = QString("%1:%2").arg(server.address).arg(server.port);
            srv.password = server.password;
            srv.rcon     = server.rcon;
            srv.profile  = DEFAULT_PROFILE_ID;

            config_.addServer(srv);
        }
    }

    if (!config_.save()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save config file."), QMessageBox::Ok);    
    }
}

void MainWindow::loadTheme(const QString& theme)
{
    if (theme != "dark" && theme != "light") {
        return;
    }

    QFile styleFile(":/themes/" + theme + "/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(styleFile.readAll());
    }

    if (theme == "dark") {
        ui_->actionDark->setChecked(true);
        ui_->actionLight->setChecked(false);
    } else if (theme == "light") {
        ui_->actionLight->setChecked(true);
        ui_->actionDark->setChecked(false);
    }
}

void MainWindow::createLanguageMenu()
{
    QActionGroup* langGroup{new QActionGroup(ui_->menuLanguage)};
    langGroup->setExclusive(true);

    QObject::connect(langGroup, &QActionGroup::triggered, this, &MainWindow::languageChanged);

    langPath_ = QApplication::applicationDirPath();
    langPath_.append("/languages");
    QStringList fileNames{QDir(langPath_).entryList(QStringList("*.qm"))};

    QString systemLocale{QLocale::system().name()};

    int langCount{fileNames.size()};
    if (langCount == 0) {
        ui_->menuLanguage->deleteLater();
    } else {
        bool     systemLocaleExsits{false};
        QAction* defaultEnglishAction{nullptr};
        for (int i{0}; i < fileNames.size(); ++i) {
            // Get locale extracted by filename.
            QString locale{fileNames[i]};
            locale.truncate(locale.lastIndexOf('.')); // Remove file extension.

            QString lang{QLocale::languageToString(QLocale(locale).language())};

            QIcon ico(QString("%1/icons/%2.svg").arg(langPath_, locale));

            QAction* action{new QAction(ico, lang, this)};
            action->setCheckable(true);
            action->setData(locale);

            ui_->menuLanguage->addAction(action);
            langGroup->addAction(action);

            // Set default translators and language checked.
            if (locale == systemLocale) {
                systemLocaleExsits = true;
                action->setChecked(true);
            } else if (locale == "en_US") {
                defaultEnglishAction = action;
            }
        }

        if (systemLocaleExsits) {
            // Load the translation the same as for the system.
            loadLanguage(systemLocale);
        } else if (defaultEnglishAction != nullptr) {
            // Set English by default.
            defaultEnglishAction->setChecked(true);
        }
    }
}

void MainWindow::switchTranslator(QTranslator& translator, const QString& filePath)
{
    // Remove the old translator.
    QCoreApplication::removeTranslator(&translator);

    // Load the new translator.
    if (translator.load(filePath)) {
        QCoreApplication::installTranslator(&translator);
    }
}

void MainWindow::loadLanguage(const QString& newLanguage)
{
    if (currLang_ != newLanguage) {
        currLang_ = newLanguage;

        QLocale::setDefault(QLocale(newLanguage));

        // Path to application translation files.
        QString langFilePath{QString("%1/%2.qm").arg(langPath_, newLanguage)};
        switchTranslator(translator_, langFilePath);

        // Path to Qt translation files.
        QString qtLangFilePath = QString("%1/../translations/qt_%2.qm")
                                     .arg(langPath_,
                                          newLanguage.mid(0, newLanguage.lastIndexOf('_')));
        switchTranslator(translatorQt_, qtLangFilePath);
    }
}

void MainWindow::languageChanged(QAction* action)
{
    if (action != nullptr) {
        loadLanguage(action->data().toString());
    }
}

void MainWindow::on_actionCheckForUpdates_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/RinatNamazov/SampX/releases"));
}

void MainWindow::on_actionSite_triggered()
{
    QDesktopServices::openUrl(QUrl("https://rinwares.com/sampx"));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("SampX About"));
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(QString(R"(<p style='text-align:center'>
<img src=':/icons/sampx_logo.png' height='100' align='center'>
<br/><br/>
%1: %2</p><br/>
SampX — %3<br/><br/>
%4: <a href='https://rinwares.com'><span style="text-decoration: none; color:white;">rinwares.com</span></a><br/>
VK: <a href='https://vk.com/rinwares'><span style="text-decoration: none; color:white;">vk.com/rinwares</span></a><br/>
TG: <a href='https://t.me/rinwares_official'><span style="text-decoration: none; color:white;">t.me/rinwares_official</span></a><br/>
Discord: <a href='https://rinwares.com/discord'><span style="text-decoration: none; color:white;">rinwares.com/discord</span></a><br/>
BlastHack: <a href='https://www.blast.hk/threads/105888/'><span style="text-decoration: none; color:white;">blast.hk/threads/105888/</span></a><br/><br/>
%5: <a href='https://t.me/Rinat_Namazov'><span style="text-decoration: none; color:white;">Rinat Namazov</span></a><br/><br/>
%6: <a href='https://rinwares.com/donate'><span style="text-decoration: none; color:white;">rinwares.com/donate</span></a><br/><br/>
Copyright © 2021 RINWARES. All rights reserved.)")
                       .arg(tr("Version"),
                            QCoreApplication::applicationVersion(),
                            tr("The custom launcher of SA-MP"),
                            tr("Site"),
                            tr("Developer"),
                            tr("Donate")));
    msgBox.exec();
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionSamp_triggered()
{
    sampVersionsDialog_->show();
}

void MainWindow::on_actionProxy_triggered()
{
    proxysDialog_->show();
}

void MainWindow::on_actionAdapter_triggered()
{
    adaptersDialog_->show();
}

void MainWindow::on_actionDark_triggered()
{
    loadTheme("dark");
    settings_->setValue("theme", "dark");
}

void MainWindow::on_actionLight_triggered()
{
    loadTheme("light");
    settings_->setValue("theme", "light");
}

void MainWindow::on_connectButton_clicked()
{
    launchGameWithServerOnRow(ui_->servers->currentRow());
}

void MainWindow::on_search_textChanged(const QString& text)
{
    if (text.isEmpty()) {
        for (int i{0}; i < ui_->servers->rowCount(); ++i) {
            ui_->servers->showRow(i);
        }
        return;
    }

    for (int i{0}; i < ui_->servers->rowCount(); ++i) {
        ui_->servers->hideRow(i);
    }

    auto items{ui_->servers->findItems(text, Qt::MatchContains)};
    for (const auto& item : items) {
        ui_->servers->showRow(item->row());
    }
}

void MainWindow::on_group_currentIndexChanged(int index)
{
    for (int i{0}; i < ui_->servers->rowCount(); ++i) {
        QTableWidgetItem* addressRow{ui_->servers->item(i, 5)};
        if (addressRow != nullptr) {
            QVariant userData{addressRow->data(Qt::UserRole)};
            if (!userData.isNull()) {
                auto sq{userData.value<SampQuery*>()};
                if (sq != nullptr) {
                    delete sq;
                }
            }
        }
    }

    ui_->servers->setRowCount(0);

    if (index == INDEX_INTERNET_GROUP) {
        sampQuery_.requestMasterServerList();
        ui_->deleteGroupButton->setEnabled(false);
        ui_->addServerButton->setEnabled(false);
    } else {
        ui_->deleteGroupButton->setEnabled(index != INDEX_DEFAULT_GROUP);
        ui_->addServerButton->setEnabled(true);

        quint32 groupId{getGroupOrProxyOrAdapterByIndex(index)};

        for (quint32 i{0}; i < config_.getServerCount(); ++i) {
            auto server{config_.getServer(i)};

            if (server.group == groupId) {
                int row{ui_->servers->rowCount()};
                ui_->servers->insertRow(row);

                QTableWidgetItem* hostnameItem{new QTableWidgetItem(tr("Not Available"))};
                hostnameItem->setData(Qt::UserRole, QVariant::fromValue(i)); // Server ID

                ui_->servers->setItem(row, 0, hostnameItem);
                ui_->servers->setItem(row, 1, new QTableWidgetItem());
                ui_->servers->setItem(row, 2, new QTableWidgetItem());
                ui_->servers->setItem(row, 3, new QTableWidgetItem());
                ui_->servers->setItem(row, 4, new QTableWidgetItem());
                ui_->servers->setItem(row, 5, new QTableWidgetItem(server.address));

                pingServerInTable(row);
            }
        }
    }

    on_search_textChanged(ui_->search->text());
}

void MainWindow::on_group_editTextChanged(const QString& text)
{
    int curGroupIndex{ui_->group->currentIndex()};
    if (curGroupIndex == -1) {
        return;
    }

    if (curGroupIndex != INDEX_INTERNET_GROUP) {
        quint32 groupId{getGroupOrProxyOrAdapterByIndex(curGroupIndex)};
        config_.setGroup(groupId, text);
        ui_->group->setItemText(curGroupIndex, text);
    } else {
        ui_->group->setEditText("Internet");
    }
}

void MainWindow::on_addGroupButton_clicked()
{
    // Generating a unique group name.
    auto [groupIndex, groupName] = [this]() -> QPair<int, QString> {
        int     index{ui_->group->count()};
        QString name;

        int i{index};
        do {
            name = QString("Group #%1").arg(i);
            ++i;
        } while (ui_->group->findText(name) != -1);

        return {index, name};
    }();

    config_.addGroup(groupName);

    ui_->group->addItem(groupName);
    ui_->group->setCurrentIndex(groupIndex);
}

void MainWindow::on_deleteGroupButton_clicked()
{
    QMessageBox::StandardButton reply
        = QMessageBox::question(this,
                                "SampX",
                                tr("Are you sure you want to delete the current group?"),
                                QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }

    int curGroupIndex{ui_->group->currentIndex()};
    // Do not delete the first two standard groups.
    if (curGroupIndex == -1 || curGroupIndex == INDEX_INTERNET_GROUP
        || curGroupIndex == INDEX_DEFAULT_GROUP) {
        return;
    }

    ui_->group->removeItem(curGroupIndex);

    config_.deleteGroup(getGroupOrProxyOrAdapterByIndex(curGroupIndex));
}

void MainWindow::on_addServerButton_clicked()
{
    bool    ok;
    QString serverAddr{QInputDialog::getText(this,
                                             tr("Add server"),
                                             tr("Enter new server host:port"),
                                             QLineEdit::Normal,
                                             "",
                                             &ok)};
    if (!ok || !isValidServerAddress(serverAddr)) {
        return;
    }

    int curGroupIndex{ui_->group->currentIndex()};
    if (curGroupIndex == INDEX_INTERNET_GROUP) {
        return;
    }

    quint32 groupId{getGroupOrProxyOrAdapterByIndex(curGroupIndex)};
    if (config_.findServerByAddress(serverAddr, groupId) != -1) {
        return;
    }

    SettingsData::Server server;
    server.group   = groupId;
    server.address = serverAddr;
    server.profile = ui_->profile->currentIndex();
    quint32 serverId{config_.addServer(server)};

    int i{ui_->servers->rowCount()};
    ui_->servers->insertRow(ui_->servers->rowCount());

    QTableWidgetItem* hostnameRow{new QTableWidgetItem()};
    hostnameRow->setData(Qt::UserRole, QVariant::fromValue(serverId));
    ui_->servers->setItem(i, 0, hostnameRow);

    ui_->servers->setItem(i, 1, new QTableWidgetItem());
    ui_->servers->setItem(i, 2, new QTableWidgetItem());
    ui_->servers->setItem(i, 3, new QTableWidgetItem());
    ui_->servers->setItem(i, 4, new QTableWidgetItem());
    ui_->servers->setItem(i, 5, new QTableWidgetItem(server.address));

    ui_->servers->setCurrentCell(i, 0);
}

void MainWindow::on_servers_cellDoubleClicked(int row, int column)
{
    if (row == -1) {
        return;
    }

    QString address, password;
    if (ui_->group->currentIndex() == INDEX_INTERNET_GROUP) {
        address = ui_->servers->item(row, 5)->text();
    } else {
        quint32 serverId{ui_->servers->item(row, 0)->data(Qt::UserRole).value<quint32>()};
        auto    srv{config_.getServer(serverId)};
        address  = srv.address;
        password = srv.password;
    }

    launchGame(address, password);
}

void MainWindow::on_servers_currentCellChanged(int currentRow,
                                               int currentColumn,
                                               int previousRow,
                                               int previousColumn)
{
    if (currentRow == -1) {
        return;
    }

    ui_->serverPing->clear();
    ui_->serverPlayers->clear();
    ui_->serverMode->clear();
    ui_->serverLanguage->clear();
    ui_->serverMap->clear();
    ui_->serverVersion->clear();
    ui_->serverWeather->clear();
    ui_->serverUrl->clear();
    ui_->serverWorldTime->clear();

    pingServerInTable(currentRow, false);

    QString serverAddress{ui_->servers->item(currentRow, 5)->text()};

    ui_->serverInfoBox->setTitle(tr("Server Information: ") + serverAddress);

    if (ui_->group->currentIndex() != INDEX_INTERNET_GROUP) {
        quint32 serverId{ui_->servers->item(currentRow, 0)->data(Qt::UserRole).value<quint32>()};

        auto srv{config_.getServer(serverId)};

        ui_->profile->setCurrentIndex(srv.profile);
    }

    ui_->connectButton->setEnabled(true);
}

void MainWindow::on_profile_currentIndexChanged(int index)
{
    if (index == -1) {
        return;
    } else if (index == DEFAULT_PROFILE_ID) {
        ui_->deleteProfileButton->setEnabled(false);
    } else {
        ui_->deleteProfileButton->setEnabled(true);
    }

    auto profile{config_.getProfile(index)};
    ui_->nickname->setText(profile.nickname);
    ui_->proxy->setCurrentIndex(getIndexOfGroupOrProxyOrAdapter(profile.proxy));
    ui_->adapter->setCurrentIndex(getIndexOfGroupOrProxyOrAdapter(profile.adapter));
    ui_->gameDir->setText(config_.getGameDir(profile.gameDir));
    ui_->gameExe->setText(config_.getGameExecutable(profile.gameExe));
    ui_->sampVersion->setCurrentIndex(profile.samp);
    ui_->comment->setPlainText(profile.comment);

    int currentRow{ui_->servers->currentRow()};
    if (currentRow != -1 && ui_->group->currentIndex() != INDEX_INTERNET_GROUP) {
        quint32 serverId{ui_->servers->item(currentRow, 0)->data(Qt::UserRole).value<quint32>()};

        auto srv{config_.getServer(serverId)};
        if (srv.profile != index) {
            srv.profile = index;
            config_.setServer(serverId, srv);
        }
    }
}

void MainWindow::on_profile_editTextChanged(const QString& text)
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1) {
        return;
    }
    if (profileId == DEFAULT_PROFILE_ID) {
        ui_->profile->setEditText("Default");
        return;
    }

    auto profile{config_.getProfile(profileId)};
    profile.name = text;
    config_.setProfile(profileId, profile);
    ui_->profile->setItemText(profileId, text);
}

void MainWindow::on_nickname_editingFinished()
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1) {
        return;
    }

    auto profile{config_.getProfile(profileId)};
    profile.nickname = ui_->nickname->text();
    config_.setProfile(profileId, profile);
}

void MainWindow::on_adapter_currentIndexChanged(int index)
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1) {
        return;
    }

    auto profile{config_.getProfile(profileId)};
    profile.adapter = getGroupOrProxyOrAdapterByIndex(index);
    config_.setProfile(profileId, profile);
}

void MainWindow::on_proxy_currentIndexChanged(int index)
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1) {
        return;
    }

    auto profile{config_.getProfile(profileId)};
    profile.proxy = getGroupOrProxyOrAdapterByIndex(index);
    config_.setProfile(profileId, profile);
}

void MainWindow::on_gameDir_editingFinished()
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1) {
        return;
    }

    auto profile{config_.getProfile(profileId)};

    QString gameDir{ui_->gameDir->text()};

    int gameDirId{config_.findGameDir(gameDir)};
    if (gameDirId != -1) {
        profile.gameDir = gameDirId;
    } else {
        quint32 lastGameDir{profile.gameDir};
        profile.gameDir = config_.addGameDir(gameDir);
        if (lastGameDir != -1) {
            config_.maybeCleanUpGameDir(lastGameDir);
        }
    }

    config_.setProfile(profileId, profile);
}

void MainWindow::on_selectGameDirButton_clicked()
{
    QString dirPath{QFileDialog::getExistingDirectory(this,
                                                      tr("Select the directory with the game"),
                                                      QDir::currentPath())};
    if (!dirPath.isEmpty()) {
        ui_->gameDir->setText(dirPath);
    }
}

void MainWindow::on_gameExe_editingFinished()
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1) {
        return;
    }

    auto profile{config_.getProfile(profileId)};

    QString gameExe{ui_->gameExe->text()};

    int gameDirId{config_.findGameDir(gameExe)};
    if (gameDirId != -1) {
        profile.gameExe = gameDirId;
    } else {
        quint32 lastGameExe{profile.gameExe};
        profile.gameExe = config_.addGameExecutable(gameExe);
        if (lastGameExe != -1) {
            config_.maybeCleanUpGameExecutable(lastGameExe);
        }
    }

    config_.setProfile(profileId, profile);
}

void MainWindow::on_sampVersion_currentIndexChanged(int index)
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1) {
        return;
    }

    auto profile{config_.getProfile(profileId)};
    profile.samp = index;
    config_.setProfile(profileId, profile);
}

void MainWindow::on_comment_textChanged()
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1) {
        return;
    }

    auto profile{config_.getProfile(profileId)};
    profile.comment = ui_->comment->toPlainText();
    config_.setProfile(profileId, profile);
}

void MainWindow::on_createNewProfileButton_clicked()
{
    // Generating a unique profile name.
    auto [profileIndex, profileName]{[this]() -> QPair<int, QString> {
        int     index{ui_->profile->count()};
        QString name;

        int i{index};
        do {
            name = QString("Profile #%1").arg(i);
            ++i;
        } while (ui_->profile->findText(name) != -1);

        return {index, name};
    }()};

    SettingsData::Profile profile;
    profile.name    = profileName;
    profile.gameDir = 0;
    profile.gameExe = 0;
    profile.samp    = 0;
    config_.addProfile(profile);

    ui_->profile->addItem(profileName);
    ui_->profile->setCurrentIndex(profileIndex);
}

void MainWindow::on_deleteProfileButton_clicked()
{
    int profileId{ui_->profile->currentIndex()};
    if (profileId == -1 || profileId == DEFAULT_PROFILE_ID) {
        return;
    }

    config_.deleteProfile(profileId);
    ui_->profile->removeItem(profileId);
}

void MainWindow::updateServerInfo()
{
    // Todo: Add this to the settings.
    /*
    // Ping only selected server.
    int row{ui_->servers->currentRow()};
    if (row != -1) {
        pingServerInTable(row);
    }
    pingTimer_->start(1000);
 */

    // Ping all servers.
    for (int i{0}; i < ui_->servers->rowCount(); ++i) {
        pingServerInTable(i);
    }
}

void MainWindow::pingServerInTable(int row, bool useHttp)
{
    // Todo: Create a queue of ping query.

    QString serverAddress{ui_->servers->item(row, 5)->text()};

    if (ui_->servers->currentRow() == row) {
        ui_->serverAddress->setText(serverAddress);
    }

    QTableWidgetItem* addressRow{ui_->servers->item(row, 5)};

    auto sq{addressRow->data(Qt::UserRole).value<SampQuery*>()};
    if (sq) {
        if (sq->getAddress() != serverAddress) {
            sq->setAddress(serverAddress);
        }
    } else {
        sq = new SampQuery(serverAddress);
        addressRow->setData(Qt::UserRole, QVariant::fromValue(sq));

        QTableWidgetItem* pingItem{ui_->servers->item(row, 2)};

        QObject::connect(sq, &SampQuery::pingCalculated, this, [this, pingItem](int ping) {
            QString pingStr{QString::number(ping)};

            pingItem->setText(pingStr);

            if (ui_->servers->item(ui_->servers->currentRow(), 2) == pingItem) {
                ui_->serverPing->setText(pingStr);
            }
        });

        QTableWidgetItem* hostnameItem{ui_->servers->item(row, 0)};
        QTableWidgetItem* onlineItem{ui_->servers->item(row, 1)};
        QTableWidgetItem* modeItem{ui_->servers->item(row, 3)};
        QTableWidgetItem* languageItem{ui_->servers->item(row, 4)};

        QObject::connect(sq,
                         &SampQuery::serverInfoResponse,
                         this,
                         [this, serverAddress, hostnameItem, onlineItem, modeItem, languageItem](
                             ServerInfoResponse si) {
                             QString playersText{
                                 QString("%1 / %2").arg(si.curPlayers).arg(si.maxPlayers)};

                             hostnameItem->setText(si.hostname);
                             onlineItem->setText(playersText);
                             modeItem->setText(si.mode);
                             languageItem->setText(si.language);

                             if (si.isClosedServer) {
                                 hostnameItem->setForeground(QBrush{ClosedServersColor});
                             }

                             if (ui_->servers->item(ui_->servers->currentRow(), 0) == hostnameItem) {
                                 ui_->serverInfoBox->setTitle(tr("Server Information: ")
                                                              + si.hostname);
                                 ui_->serverPlayers->setText(playersText);
                                 ui_->serverMode->setText(si.mode);
                                 ui_->serverLanguage->setText(si.language);
                             }
                         });

        QObject::connect(sq,
                         &SampQuery::serverRulesResponse,
                         this,
                         [this, hostnameItem](QMap<QString, QString> rules) {
                             if (ui_->servers->item(ui_->servers->currentRow(), 0) != hostnameItem) {
                                 return;
                             }

                             ui_->serverMap->setText("-");
                             ui_->serverVersion->setText("-");
                             ui_->serverWeather->setText("-");
                             ui_->serverUrl->setText("-");
                             ui_->serverWorldTime->setText("-");

                             for (auto it{rules.cbegin()}; it != rules.cend(); ++it) {
                                 QString key{it.key()};
                                 QString value{it.value()};

                                 if (key == "mapname") {
                                     ui_->serverMap->setText(value);
                                 } else if (key == "version") {
                                     ui_->serverVersion->setText(value);
                                 } else if (key == "weather") {
                                     ui_->serverWeather->setText(value);
                                 } else if (key == "weburl") {
                                     ui_->serverUrl->setText(
                                         "<a href=\"http://" + value
                                         + "\"><span style=\"text-decoration: none; color:white;\">"
                                         + value + "</span></a>");
                                 } else if (key == "worldtime") {
                                     ui_->serverWorldTime->setText(value);
                                 }
                             }
                         });
    }

    if (useHttp) {
        //sq->requestHttpPing();
    }

    for (int i{0}; i < 5; ++i) {
        sq->requestPing();
        sq->requestInformation();
        sq->requestRules();
    }
}

void MainWindow::launchGameWithServerOnRow(int row)
{
    if (row == -1) {
        return;
    }

    QString address, password;
    if (ui_->group->currentIndex() == INDEX_INTERNET_GROUP) {
        address = ui_->servers->item(row, 5)->text();
    } else {
        quint32 serverId{ui_->servers->item(row, 0)->data(Qt::UserRole).value<quint32>()};
        auto    srv{config_.getServer(serverId)};

        address  = srv.address;
        password = srv.password;
    }

    launchGame(address, password);
}

void MainWindow::launchGame(const QString& address, const QString& password)
{
    QString nickName{ui_->nickname->text()};
    if (nickName.isEmpty()) {
        QMessageBox::warning(this, "SampX", tr("You have an empty nickname."));
        return;
    }

    QString gameDir{ui_->gameDir->text()};
    if (gameDir.isEmpty()) {
        QMessageBox::warning(this, "SampX", tr("You have an empty game directory."));
        return;
    }

    QString gameExe{ui_->gameExe->text()};
    if (gameExe.isEmpty()) {
        QMessageBox::warning(this, "SampX", tr("You have an empty game executable."));
        return;
    }

    int sampVersionId{ui_->sampVersion->currentIndex()};
    if (sampVersionId == -1) {
        QMessageBox::warning(this, "SampX", tr("You have an empty samp version."));
        return;
    }

    gameLauncher_.setAddress(address);
    gameLauncher_.setPassword(password);

    gameLauncher_.setNickName(nickName);

    gameLauncher_.setGameDir(gameDir);
    gameLauncher_.setGameExe(gameExe);

    quint32 adapterId{getGroupOrProxyOrAdapterByIndex(ui_->adapter->currentIndex())};
    gameLauncher_.setNetworkAdapter(adapterId == -1 ? "" : config_.getAdapter(adapterId));

    quint32 proxyId{getGroupOrProxyOrAdapterByIndex(ui_->proxy->currentIndex())};
    gameLauncher_.setProxy(proxyId == -1 ? QPair<QString, QString>{QString{}, QString{}}
                                         : config_.getProxy(proxyId));

    QString sampDllName{config_.getSampVersion(sampVersionId).second};
    gameLauncher_.replaceSampLibrary(sampDllName);

    gameLauncher_.launch();
}
