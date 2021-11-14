/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           AdaptersDialog.hpp
 *  DESCRIPTION:    Network adapters dialog
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;

namespace Ui {
class AdaptersDialog;
}
QT_END_NAMESPACE

class SettingsData;

class AdaptersDialog : public QDialog {
    Q_OBJECT

public:
    explicit AdaptersDialog(SettingsData* config, QComboBox* comboBox, QWidget* parent = nullptr);
    ~AdaptersDialog();

private slots:
    void on_addButton_clicked();
    void on_deleteButton_clicked();

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_adapters_currentCellChanged(int currentRow,
                                        int currentColumn,
                                        int previousRow,
                                        int previousColumn);

private:
    Ui::AdaptersDialog* ui_;
    SettingsData*       config_;
    QComboBox*          combo_;

    void addAdaptersFromConfig();
    void updateCombo();
};
