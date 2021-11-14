/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           ProxysDialog.cpp
 *  DESCRIPTION:    Proxys dialog
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#include <QComboBox>

#include "ProxysDialog.hpp"

#include "SettingsData.hpp"
#include "ui_ProxysDialog.h"

ProxysDialog::ProxysDialog(SettingsData* config, QComboBox* comboBox, QWidget* parent)
    : QDialog(parent, Qt::WindowCloseButtonHint)
    , ui_(new Ui::ProxysDialog)
    , config_(config)
    , combo_(comboBox)
{
    ui_->setupUi(this);
    addProxysFromConfig();
}

ProxysDialog::~ProxysDialog()
{
    delete ui_;
}

void ProxysDialog::on_addButton_clicked()
{
    int row{ui_->proxys->rowCount()};
    ui_->proxys->insertRow(row);
    ui_->proxys->setItem(row, 0, new QTableWidgetItem());
    ui_->proxys->setItem(row, 1, new QTableWidgetItem());
}

void ProxysDialog::on_deleteButton_clicked()
{
    int row{ui_->proxys->currentRow()};
    if (row == -1) {
        return;
    }
    ui_->proxys->removeRow(row);
}

void ProxysDialog::on_buttonBox_accepted()
{
    quint32 proxyCount{config_->getProxyCount()};
    int    rowCount{ui_->proxys->rowCount()};

    for (quint32 i{static_cast<quint32>(rowCount)}; i < proxyCount; ++i) {
        config_->deleteProxy(i);
    }

    for (int i{0}; i < rowCount; ++i) {
        QString                       proxyAddress{ui_->proxys->item(i, 0)->text()};
        QString                       proxyKey{ui_->proxys->item(i, 1)->text()};
        const QPair<QString, QString> proxy{proxyAddress, proxyKey};
        if (i < proxyCount) {
            config_->setProxy(i, proxy);
        } else {
            config_->addProxy(proxy);
        }
    }

    updateCombo();
}

void ProxysDialog::on_buttonBox_rejected()
{
    addProxysFromConfig();
}

void ProxysDialog::on_proxys_currentCellChanged(int currentRow,
                                                int currentColumn,
                                                int previousRow,
                                                int previousColumn)
{
    if (currentRow == -1) {
        ui_->deleteButton->setEnabled(false);
    } else {
        ui_->deleteButton->setEnabled(true);
    }
}

void ProxysDialog::addProxysFromConfig()
{
    quint32 proxyCount{config_->getProxyCount()};

    ui_->proxys->setRowCount(proxyCount);

    for (quint32 i{0}; i < proxyCount; ++i) {
        auto proxy{config_->getProxy(i)};

        QTableWidgetItem* addressItem{ui_->proxys->item(i, 0)};
        if (addressItem != nullptr) {
            addressItem->setText(proxy.first);
        } else {
            ui_->proxys->setItem(i, 0, new QTableWidgetItem(proxy.first));
        }

        QTableWidgetItem* keyItem{ui_->proxys->item(i, 1)};
        if (keyItem != nullptr) {
            keyItem->setText(proxy.second);
        } else {
            ui_->proxys->setItem(i, 1, new QTableWidgetItem(proxy.second));
        }
    }

    updateCombo();
}

void ProxysDialog::updateCombo()
{
    combo_->clear();
    combo_->addItem("No");
    for (quint32 i{0}; i < config_->getProxyCount(); ++i) {
        auto proxy{config_->getProxy(i)};
        combo_->addItem(proxy.first);
    }
}
