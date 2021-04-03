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
#include "rx_tx_threads.h"

rx_thread::rx_thread(){
    upload_operation = 0;
    rx_buff_size = 0;    
    serial_port = NULL;
}

void rx_thread::set_xtal_freq(QString *freq){
    xtal_freq = *freq;
}

void rx_thread::on_upload_operation(bool upld_opr){
    upload_operation = upld_opr;
    //qDebug()<<"upload_operation = "<<upload_operation;
}

void rx_thread::open_port(const QString *port_name, PortSettings *port_settings){
    serial_port = new QextSerialPort(*port_name, QextSerialPort::EventDriven);

    serial_port->setBaudRate(port_settings->BaudRate);
    serial_port->setParity(port_settings->Parity);
    serial_port->setDataBits(port_settings->DataBits);
    serial_port->setStopBits(port_settings->StopBits);
    serial_port->setFlowControl(port_settings->FlowControl);
    serial_port->setTimeout(port_settings->Timeout_Millisec);

    if (serial_port->open(QIODevice::ReadWrite) == true) {
        connect(serial_port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        qDebug() << "(lpcflashapp) (rx_thread) Listening for data on: " << serial_port->portName()<<"!";

        emit port_is_open();
    }
    else {
        QString temp = serial_port->errorString();
        errors(DEVICE_OPEN_FAILED, &temp);
    }
}

void rx_thread::close_port(){    
    if(serial_port){
        qDebug()<<"(lpcflashapp) (rx_thread) Closing serial port ...";
        serial_port->close();
        delete serial_port;
        serial_port = NULL;
    }
}

rx_thread::~rx_thread(){    
    close_port();
}

void rx_thread::on_disconnect(){
    close_port();
}

void rx_thread::run(){
    qDebug()<<"(lpcflashapp) (rx_thread) Starting...";
}

void rx_thread::onReadyRead(){
    QString rx_buff_str;
    unsigned long isp_error = 1;
    QByteArray temp_buff;
    unsigned long bytes_recv;

    bytes_recv = serial_port->bytesAvailable();
    rx_buff_size += bytes_recv;
    temp_buff.resize(bytes_recv);
    serial_port->read(temp_buff.data(), temp_buff.size());

    //qDebug()<<"rx[*] "<<temp_buff;

    rx_buff += temp_buff;

    if(rx_buff.size() >= 2){
        if((rx_buff.at(rx_buff_size-2) == '\r') && (rx_buff.at(rx_buff_size-1) == '\n')){

            rx_buff_str = rx_buff;
            rx_buff_str = rx_buff_str.section("\r\n", 1, -1);
            int carr_return = rx_buff.count('\r');

            if((carr_return >= 2) && (upload_operation != 1)){

                QString rx_buff_str2;
                rx_buff_str2 = rx_buff;
                rx_buff_str2 = rx_buff_str2.section("\r\n", 0, -3);
                rx_buff = rx_buff_str.toLocal8Bit();
                rx_buff_size = rx_buff.size();

                if(rx_buff_str2 == xtal_freq){
                    isp_error = CMD_SUCCESS;
                }
                else{
                    isp_error = convert_isp_error_code(&rx_buff_str2);
                }
            }
            else{
                if(upload_operation){
                    isp_error = CMD_SUCCESS;
                }
                else{
                    isp_error = convert_isp_error_code(&rx_buff_str);
                }
            }

            rx_buff.remove(rx_buff_size-2, 2);

            emit change_download_stage(isp_error, rx_buff);
            rx_buff.clear();
            rx_buff_size = 0;
        }
    }
}

void rx_thread::on_send_serial_data(QByteArray *bin_data){    
    serial_port->write(*bin_data);    
}

unsigned long rx_thread::convert_isp_error_code(QString *isp_err){
    bool ok;
    unsigned long tmp = 0;

    if((*isp_err == "Synchronized") ||
       (*isp_err == "OK") ||
       (*isp_err == "A 0") ||
       isp_err->isEmpty()){
        return CMD_SUCCESS;
    }

    tmp = isp_err->toInt(&ok, 10);

    if(!ok){
        QString tmp_str("convert_isp_error_code");
        //errors(CONVERSION_FAILED, &tmp_str);
    }
    else{
        switch(tmp){
        case CMD_SUCCESS:
            qDebug()<<"(lpcflashapp) (rx_thread) CMD_SUCCESS";
            break;
        case INVALID_COMMAND:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: INVALID_COMMAND";
            break;
        case SRC_ADDR_ERROR:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: SRC_ADDR_ERROR";
            break;
        case DST_ADDR_ERRPR:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: DST_ADDR_ERRPR";
            break;
        case SRC_ADDR_NOT_MAPPED:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: SRC_ADDR_NOT_MAPPED";
            break;
        case DST_ADDR_NOT_MAPPED:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: DST_ADDR_NOT_MAPPED";
            break;
        case COUNT_ERROR:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: COUNT_ERROR";
            break;
        case INVALID_SECTOR:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: INVALID_SECTOR";
            break;
        case SECTOR_NOT_BLANK:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: SECTOR_NOT_BLANK";
            break;
        case SECTOR_NOT_PREPARED_WR:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: SECTOR_NOT_PREPARED_WR";
            break;
        case COMPARE_ERROR:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: COMPARE_ERROR";
            break;
        case BUSY:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: BUSY";
            break;
        case PARAM_ERROR:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: PARAM_ERROR";
            break;
        case ADDR_ERROR:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: ADDR_ERROR";
            break;
        case ADDR_NOT_MAPPED:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: ADDR_NOT_MAPPED";
            break;
        case CMD_LOCKED:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: CMD_LOCKED";
            break;
        case INVALID_CODE:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: INVALID_CODE";
            break;
        case INVALID_BAUD_RATE:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: INVALID_BAUD_RATE";
            break;
        case INVALID_STOP_BIT:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: INVALID_STOP_BIT";
            break;
        case CODE_READ_PROTECTION_ENABLED:
            qDebug()<<"(lpcflashapp) (rx_thread) ERROR: CODE_READ_PROTECTION_ENABLED";
            break;

        default:
            qDebug()<<"(lpcflashapp) (rx_thread) Unknown error!";
            break;
        }
    }

    return tmp;
}
