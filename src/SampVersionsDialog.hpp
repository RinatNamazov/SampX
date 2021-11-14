/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SampVersionsDialog.hpp
 *  DESCRIPTION:    Samp versions dialog
 *  COPYRIGHT:      (c) 2021 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;

namespace Ui {
class SampVersionsDialog;
}
QT_END_NAMESPACE

class SettingsData;

class SampVersionsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SampVersionsDialog(SettingsData* config,
                                QComboBox*    comboBox,
                                QWidget*      parent = nullptr);
    ~SampVersionsDialog();

private slots:
    void on_addButton_clicked();
    void on_deleteButton_clicked();

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_versions_currentCellChanged(int currentRow,
                                        int currentColumn,
                                        int previousRow,
                                        int previousColumn);

private:
    Ui::SampVersionsDialog* ui_;
    SettingsData*           config_;
    QComboBox*              combo_;

    void addServersFromConfig();
    void updateCombo();
};
