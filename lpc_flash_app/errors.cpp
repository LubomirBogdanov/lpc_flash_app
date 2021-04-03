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
#include "errors.h"

void errors(unsigned long err_num, QString *add_string){
    QString err_msg;

    switch(err_num){
    case CANT_OPEN_FILE:
        err_msg = "(lpcflashapp) ERROR: Can't open file: ";
        err_msg += *add_string;
        err_msg += "!";
        qDebug()<<err_msg;
        break;
    case CANT_WRITE_FILE:
        err_msg = "(lpcflashapp) ERROR: Can't write to file: ";
        err_msg += *add_string;
        err_msg += "!";
        qDebug()<<err_msg;
        break;
    case PORT_ERR:
        err_msg = "(lpcflashapp) ERROR: port number wrong! ";
        err_msg += add_string;
        qDebug()<<err_msg;
    case CONVERSION_FAILED:
        err_msg = "(lpcflashapp) ERROR: number conversion failed! ";                    
        err_msg += *add_string;
        qDebug()<<err_msg;
        break;
    case DEVICE_OPEN_FAILED:
        err_msg = "(lpcflashapp) ERROR: device open failed: ";
        err_msg += add_string;
        qDebug()<<err_msg;
        break;
    case WRONG_STAGE:
        err_msg = "(lpcflashapp) ERROR: wrong download stage: ";
        err_msg += add_string;
        qDebug()<<err_msg;
        break;
    case BINARY_FILE_EMPTY:
        err_msg = "(lpcflashapp) ERROR: binary file empty!";
        qDebug()<<err_msg;
        break;
    case TARGET_NOT_FOUND:
        err_msg = "(lpcflashapp) ERROR: target ";
        err_msg += add_string;
        err_msg += " not found!";
        qDebug()<<err_msg;
        break;
    case CHECKSUM_FAIL:
        err_msg = "(lpcflashapp) ERROR: wrong checksum! ";
        err_msg += add_string;
        qDebug()<<err_msg;
        break;
    case VERIFY_FAIL:
        err_msg = "(lpcflashapp) ERROR: verify mismatch ";
        err_msg += add_string;
        qDebug()<<err_msg;
        break;
    case FLASH_WRITE_FAILED:
        err_msg = "(lpcflashapp) ERROR: flash write failed! ";
        err_msg += add_string;
        qDebug()<<err_msg;
        break;
    default:
        break;
    }
}
