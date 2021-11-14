/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           AdaptersDialog.cpp
 *  DESCRIPTION:    Network adapters dialog
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#include <QComboBox>

#include "AdaptersDialog.hpp"

#include "SettingsData.hpp"
#include "ui_AdaptersDialog.h"

AdaptersDialog::AdaptersDialog(SettingsData* config, QComboBox* comboBox, QWidget* parent)
    : QDialog(parent, Qt::WindowCloseButtonHint)
    , ui_(new Ui::AdaptersDialog)
    , config_(config)
    , combo_(comboBox)
{
    ui_->setupUi(this);
    addAdaptersFromConfig();
}

AdaptersDialog::~AdaptersDialog()
{
    delete ui_;
}

void AdaptersDialog::on_addButton_clicked()
{
    int row{ui_->adapters->rowCount()};
    ui_->adapters->insertRow(row);
    ui_->adapters->setItem(row, 0, new QTableWidgetItem());
}

void AdaptersDialog::on_deleteButton_clicked()
{
    int row{ui_->adapters->currentRow()};
    if (row == -1) {
        return;
    }
    ui_->adapters->removeRow(row);
}

void AdaptersDialog::on_buttonBox_accepted()
{
    quint32 adaptersCount{config_->getAdapterCount()};
    int rowCount{ui_->adapters->rowCount()};

    for (int i{rowCount}; i < adaptersCount; ++i) {
        config_->deleteAdapter(i);
    }

    for (int i{0}; i < rowCount; ++i) {
        QString adapter{ui_->adapters->item(i, 0)->text()};
        if (i < adaptersCount) {
            config_->setAdapter(i, adapter);
        } else {
            config_->addAdapter(adapter);
        }
    }

    updateCombo();
}

void AdaptersDialog::on_buttonBox_rejected()
{
    addAdaptersFromConfig();
}

void AdaptersDialog::on_adapters_currentCellChanged(int currentRow,
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

void AdaptersDialog::addAdaptersFromConfig()
{
    quint32 adaptersCount{config_->getAdapterCount()};

    ui_->adapters->setRowCount(adaptersCount);

    for (quint32 i{0}; i < adaptersCount; ++i) {
        auto adapter{config_->getAdapter(i)};

        QTableWidgetItem* adapterItem{ui_->adapters->item(i, 0)};
        if (adapterItem != nullptr) {
            adapterItem->setText(adapter);
        } else {
            ui_->adapters->setItem(i, 0, new QTableWidgetItem(adapter));
        }
    }

    updateCombo();
}

void AdaptersDialog::updateCombo()
{
    combo_->clear();
    combo_->addItem("Default");
    for (quint32 i{0}; i < config_->getAdapterCount(); ++i) {
        auto adapter{config_->getAdapter(i)};
        combo_->addItem(adapter);
    }
}
