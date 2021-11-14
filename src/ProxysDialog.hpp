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

#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;

namespace Ui {
class ProxysDialog;
}
QT_END_NAMESPACE

class SettingsData;

class ProxysDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProxysDialog(SettingsData* config, QComboBox* comboBox, QWidget* parent = nullptr);
    ~ProxysDialog();

private slots:
    void on_addButton_clicked();
    void on_deleteButton_clicked();

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_proxys_currentCellChanged(int currentRow,
                                      int currentColumn,
                                      int previousRow,
                                      int previousColumn);

private:
    Ui::ProxysDialog* ui_;
    SettingsData*     config_;
    QComboBox*        combo_;

    void addProxysFromConfig();
    void updateCombo();
};
