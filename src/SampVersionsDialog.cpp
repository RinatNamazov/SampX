/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SampVersionsDialog.cpp
 *  DESCRIPTION:    Samp versions dialog
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#include <QComboBox>

#include "SampVersionsDialog.hpp"

#include "SettingsData.hpp"
#include "ui_SampVersionsDialog.h"

SampVersionsDialog::SampVersionsDialog(SettingsData* config, QComboBox* comboBox, QWidget* parent)
    : QDialog(parent, Qt::WindowCloseButtonHint)
    , ui_(new Ui::SampVersionsDialog)
    , config_(config)
    , combo_(comboBox)
{
    ui_->setupUi(this);
    addServersFromConfig();
}

SampVersionsDialog::~SampVersionsDialog()
{
    delete ui_;
}

void SampVersionsDialog::on_addButton_clicked()
{
    int row{ui_->versions->rowCount()};
    ui_->versions->insertRow(row);
    ui_->versions->setItem(row, 0, new QTableWidgetItem());
    ui_->versions->setItem(row, 1, new QTableWidgetItem());
}

void SampVersionsDialog::on_deleteButton_clicked()
{
    int row{ui_->versions->currentRow()};
    if (row == -1) {
        return;
    }
    ui_->versions->removeRow(row);
}

void SampVersionsDialog::on_buttonBox_accepted()
{
quint32 sampVersionCount{config_->getSampVersionCount()};
    int rowCount{ui_->versions->rowCount()};

    for (int i{rowCount}; i < sampVersionCount; ++i) {
        config_->deleteSampVersion(i);
    }

    for (int i{0}; i < rowCount; ++i) {
        QString versionName{ui_->versions->item(i, 0)->text()};
        QString fileName{ui_->versions->item(i, 1)->text()};
        if (i < sampVersionCount) {
            config_->setSampVersion(i, {versionName, fileName});
        } else {
            config_->addSampVersion({versionName, fileName});
        }
    }

    updateCombo();
}

void SampVersionsDialog::on_buttonBox_rejected()
{
    addServersFromConfig();
}

void SampVersionsDialog::on_versions_currentCellChanged(int currentRow,
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

void SampVersionsDialog::addServersFromConfig()
{
    quint32 sampVersionCount{config_->getSampVersionCount()};

    ui_->versions->setRowCount(sampVersionCount);

    for (quint32 i{0}; i < sampVersionCount; ++i) {
        auto sv{config_->getSampVersion(i)};

        QTableWidgetItem* versionNameItem{ui_->versions->item(i, 0)};
        if (versionNameItem != nullptr) {
            versionNameItem->setText(sv.first);
        } else {
            ui_->versions->setItem(i, 0, new QTableWidgetItem(sv.first));
        }

        QTableWidgetItem* fileNameItem{ui_->versions->item(i, 1)};
        if (fileNameItem != nullptr) {
            fileNameItem->setText(sv.first);
        } else {
            ui_->versions->setItem(i, 1, new QTableWidgetItem(sv.second));
        }
    }

    updateCombo();
}

void SampVersionsDialog::updateCombo()
{
    combo_->clear();
    for (quint32 i{0}; i < config_->getSampVersionCount(); ++i) {
        auto sv{config_->getSampVersion(i)};
        combo_->addItem(sv.first);
    }
}
