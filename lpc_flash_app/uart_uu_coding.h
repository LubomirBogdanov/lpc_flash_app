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
#ifndef UART_UU_CODING_H
#define UART_UU_CODING_H

#include <QString>
#include <QDebug>
#include <QByteArray>
#include <iostream>
#include <iomanip>
using namespace std;

void convert_eight_to_six(QByteArray *input, QByteArray *output);
void convert_six_to_eight(QByteArray *input, QByteArray *output);
void convert_six_add_twenty(QByteArray *input, QByteArray *output);
void convert_six_subtract_twenty(QByteArray *input, QByteArray *output);
unsigned long encode_UU(QByteArray *input, QByteArray *output);
unsigned long decode_UU(QByteArray *input, QByteArray *output);
QByteArray calculate_checksum(QByteArray *input);
bool validate_checksum(QByteArray *input, unsigned long input_checksum);
QByteArray synthesize_UU_line(QByteArray *tx_data);

//For debugging purposes
void display_byte_array(QString name, QByteArray my_arr);
void hex_dump_byte_array(QByteArray byte_arr);

#endif // UART_UU_CODING_H
