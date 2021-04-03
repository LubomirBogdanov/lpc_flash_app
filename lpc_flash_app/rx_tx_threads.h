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
#ifndef RX_TX_THREADS_H
#define RX_TX_THREADS_H

#include <QDebug>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QApplication>

#include "qextserialport.h"
#include "errors.h"
#include "uart_uu_coding.h"
#include "version.h"

#include <iostream>
using namespace std;

#define SRAM_START_ADDR         0x10000400  //SRAM address where binary will be buffered before it is transferred to flash.
#define NUM_OF_SECTORS          16          //MCU specific. 16 for LPC1347
#define FLASH_SECTOR_SIZE       4096        //MCU soecific. 16 for LPC1347
#define TX_BUFF                 32          //Data written in SRAM, then transferred to FLASH.
#define MAX_UU_LINES            1           //Max number of UU lines. After sending MAX_UU_LINES the host should send checksum
#define MAX_UU_DATA             32          //Max data per UU line. Each line starts with n+32 and is terminated with \r\n.
                                            //n is MAX_UU_DATA. Total transfered bytes = [1][MAX_UU_DATA][\r][\n] [decimal value of checksum]
#define UPLOAD_UU_LINES         20          //The number of UU lines sent by the LPC to the host before a checksum is sent.
#define UPLOAD_BYTES_PER_LINE   45          //The number of bytes in a UU line sent by the LPC to the host

enum ISP_ERROR_CODES{
    CMD_SUCCESS,
    INVALID_COMMAND,
    SRC_ADDR_ERROR,
    DST_ADDR_ERRPR,
    SRC_ADDR_NOT_MAPPED,
    DST_ADDR_NOT_MAPPED,
    COUNT_ERROR,
    INVALID_SECTOR,
    SECTOR_NOT_BLANK,
    SECTOR_NOT_PREPARED_WR,
    COMPARE_ERROR,
    BUSY,
    PARAM_ERROR,
    ADDR_ERROR,
    ADDR_NOT_MAPPED,
    CMD_LOCKED,
    INVALID_CODE,
    INVALID_BAUD_RATE,
    INVALID_STOP_BIT,
    CODE_READ_PROTECTION_ENABLED
};

unsigned long encode_UU(QByteArray *input, QByteArray *output);
QByteArray calculate_checksum(QByteArray *input);
bool validate_checksum(QByteArray *input, unsigned long checksum);

void display_byte_array(QString name, QByteArray my_arr);

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
class tx_thread : public QThread{    
    Q_OBJECT

    friend unsigned long encode_UU(QByteArray *input, QByteArray *output);
    friend QByteArray calculate_checksum(QByteArray *input);
    friend void display_byte_array(QString name, QByteArray my_arr);
    friend unsigned long decode_UU(QByteArray *input, QByteArray *output);
    friend bool validate_checksum(QByteArray *input, unsigned long checksum);
    friend void hex_dump_byte_array(QByteArray byte_arr);
    friend QByteArray synthesize_UU_line(QByteArray *tx_data);

private:
    QByteArray tx_buff;
    QByteArray bin_contents;
    int bin_contents_size;
    QByteArray bin_contents_received;
    int bin_contents_received_size;
    QString xtal_freq;
    QString supplied_target_name;
    unsigned long supplied_target_id;
    bool verify;
    unsigned long num_of_sectors_to_upload;

    unsigned long get_part_id(QString *mcu);
    QString get_part_id_name(unsigned long target_id);    
    unsigned long sector_to_flash_address(unsigned long flash_sector);

public:
    unsigned long download_stage;

    void run();
    tx_thread();
    void set_target_name(QString *mcu);
    void set_binary_data(QByteArray *bin_file_data);
    void set_xtal_freq(QString *freq);
    void set_verify(bool vrfy);
    void set_stage(unsigned long stg);

signals:
    void send_serial_data(QByteArray *bin_data);
    void upload_complete(QByteArray bin_data);
    void upload_operation(bool upld_opr);
    void flash_write_complete(void);
    void erase_complete(void);

private slots:
    void on_change_download_stage(unsigned long cmd_return_code, QByteArray rx_data);    
    void on_upload_flash(unsigned long cmd_return_code, QByteArray rx_data);
    void on_erase_entire_flash(unsigned long cmd_return_code, QByteArray rx_data);
    void on_set_num_of_sectors_to_upload(unsigned long num_of_sectrs);
    void on_reset_target();
};

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
class rx_thread : public QThread{
    Q_OBJECT

friend void errors(unsigned long err_num, QString *add_string);
friend unsigned long encode_UU(QByteArray *input, QByteArray *output);

private:
    QextSerialPort *serial_port;
    QByteArray rx_buff;
    unsigned long rx_buff_size;
    QString xtal_freq;
    bool upload_operation;

    unsigned long convert_isp_error_code(QString *isp_err);

public:
    rx_thread();
    void open_port(const QString *port_name, PortSettings *port_settings);
    ~rx_thread();
    void run();
    void close_port();
    void set_xtal_freq(QString *freq);

signals:
    void port_is_open();
    void change_download_stage(unsigned long cmd_return_code, QByteArray rx_data);

private slots:
        void on_disconnect();
        void onReadyRead();
        void on_send_serial_data(QByteArray *bin_data);
        void on_upload_operation(bool upld_opr);
};

#endif // RX_TX_THREADS_H
