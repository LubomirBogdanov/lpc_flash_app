/*
    Copyright (C) 2021 Lubomir Bogdanov
    Contributor Lubomir Bogdanov <lbogdanov@tu-sofia.bg>
    This file is part of lpc_flash_app.
    lpc_flash_app is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    lpc_flash_app is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with lpc_flash_app.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LPC_FLASH_APP_H
#define LPC_FLASH_APP_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>

#include "version.h"
#include "gui_fileio.h"
#include "rx_tx_threads.h"
#include "qextserialport.h"


#include <iostream>
#include <iomanip>
using namespace std;


namespace Ui {
class lpc_flash_app;
}

typedef struct{
    QString port;
    QString mcu;
    unsigned long baud;
    unsigned char databits;
    QString parity;
    float stopbits;
    QString binary_file_path;
    QString upload_file_path;
    QString xtal_freq;
    bool add_valid_code_signature;
    bool verify;
}gui_defaults_t;

class lpc_flash_app : public QMainWindow{
    Q_OBJECT

public:
    explicit lpc_flash_app(QWidget *parent = 0);
    ~lpc_flash_app();

    void init_defaults(gui_defaults_t *defs);
    void read_config();
    void set_gui_params(gui_defaults_t *params);
    void read_binary(QString *new_binary_path);

    unsigned long convert_mcu_to_index(QString *mcu_name);
    unsigned long convert_baud_to_index(unsigned long baud);
    unsigned long convert_databits_to_index(unsigned char databits_num);
    unsigned long convert_parity_to_index(QString *par);
    unsigned long convert_stopbits_to_index(float stopbits_num);

    BaudRateType convert_baud_to_qextserial(unsigned long baud);
    DataBitsType convert_databits_to_qextserial(unsigned char databits_num);
    ParityType convert_parity_to_qextserial(QString *par);
    StopBitsType convert_stopbits_to_qextserial(float stopbits_num);

private slots:
    void on_connect_button_pressed();
    void on_actionExit_triggered();
    void on_disconnect_button_pressed();
    void on_download_button_pressed();
    void on_browse_file_pressed();
    void on_pushButton_reset_pressed();
    void on_checkBox_valid_code_clicked(bool checked);
    void on_checkBox_verify_clicked(bool checked);
    void on_pushButton_upload_pressed();
    void on_upload_complete(QByteArray bin_data);
    void on_erase_button_pressed();

    void on_actionAbout_triggered();

    void on_actionInfo_triggered();

public slots:
    void on_port_is_open();
    void on_flash_write_complete(void);
    void on_write_uploaded_binary(QByteArray bin_data);
    void on_verify_binaries(QByteArray);
    void on_erase_complete(void);

signals:
    void disconnect_button_pressed();
    void reset_target();
    void upload_flash(unsigned long par1, QByteArray par2);
    void set_num_of_sectors_to_upload(unsigned long num_of_sctr);
    void erase_flash(unsigned long, QByteArray);

private:
    Ui::lpc_flash_app *ui;
    gui_fileio config_file;
    gui_fileio binary_file;
    gui_defaults_t defaults;

    QByteArray bin_contents;
    QByteArray uploaded_bin_contents;
    tx_thread *serial_tx_thread;
    rx_thread *serial_rx_thread;    
};

#endif // LPC_FLASH_APP_H
