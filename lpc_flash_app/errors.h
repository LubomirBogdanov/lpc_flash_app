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
#ifndef ERRORS_H
#define ERRORS_H

#include <QString>
#include <QDebug>

enum {
    CONVERSION_FAILED,    
    CANT_OPEN_FILE,
    CANT_WRITE_FILE,    
    PORT_ERR,
    DEVICE_OPEN_FAILED,
    WRONG_STAGE,
    BINARY_FILE_EMPTY,
    TARGET_NOT_FOUND,
    CHECKSUM_FAIL,
    VERIFY_FAIL,
    FLASH_WRITE_FAILED,
    FLASH_ERASE_FAILED
};

void errors(unsigned long err_num, QString *add_string);

#endif // ERRORS_H
