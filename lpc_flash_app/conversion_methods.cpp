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
#include "lpc_flash_app.h"

unsigned long lpc_flash_app::convert_mcu_to_index(QString *mcu_name){
    if(*mcu_name == "lpc1315"){
        return 0;
    }

    if(*mcu_name == "lpc1316"){
        return 1;
    }

    if(*mcu_name == "lpc1317"){
        return 2;
    }

    if(*mcu_name == "lpc1345"){
        return 3;
    }

    if(*mcu_name == "lpc1346"){
        return 4;
    }

    if(*mcu_name == "lpc1347"){
        return 5;
    }

    return 0;
}

unsigned long lpc_flash_app::convert_baud_to_index(unsigned long baud){
    if(baud == 2400){
        return 0;
    }

    if(baud == 4800){
        return 1;
    }

    if(baud == 9600){
        return 2;
    }

    if(baud == 19200){
        return 3;
    }

    if(baud == 38400){
        return 4;
    }

    if(baud == 57600){
        return 5;
    }

    if(baud == 115200){
        return 6;
    }

    return 0;
}

unsigned long lpc_flash_app::convert_databits_to_index(unsigned char databits_num){
    if(databits_num == 5){
        return 0;
    }

    if(databits_num == 6){
        return 1;
    }

    if(databits_num == 7){
        return 2;
    }

    if(databits_num == 8){
        return 3;
    }

    return 0;
}

unsigned long lpc_flash_app::convert_parity_to_index(QString *par){
    if(*par == "None"){
        return 0;
    }

    if(*par == "Odd"){
        return 1;
    }

    if(*par == "Even"){
        return 2;
    }

    if(*par == "Space"){
        return 3;
    }

    if(*par == "Mark"){
        return 4;
    }

    return 0;
}

unsigned long lpc_flash_app::convert_stopbits_to_index(float stopbits_num){
    if(stopbits_num == 1.0){
        return 0;
    }

    if(stopbits_num == 1.5){
        return 1;
    }

    if(stopbits_num == 2.0){
        return 2;
    }

    return 0;
}

BaudRateType lpc_flash_app::convert_baud_to_qextserial(unsigned long baud){
    BaudRateType qext_baud;

    switch(baud){
    case 2400:
        qext_baud = BAUD2400;
        break;
    case 4800:
        qext_baud = BAUD4800;
        break;
    case 9600:
        qext_baud = BAUD9600;
        break;
    case 19200:
        qext_baud = BAUD19200;
        break;
    case 38400:
        qext_baud = BAUD38400;
        break;
    case 57600:
        qext_baud = BAUD57600;
        break;
    case 115200:
        qext_baud = BAUD115200;
        break;
    default:
        qext_baud = BAUD115200;
        break;
    }

    return qext_baud;
}

DataBitsType lpc_flash_app::convert_databits_to_qextserial(unsigned char databits_num){
    DataBitsType qext_databits;

    switch(databits_num){
    case 5:
        qext_databits = DATA_5;
        break;
    case 6:
        qext_databits = DATA_6;
        break;
    case 7:
        qext_databits = DATA_7;
        break;
    case 8:
        qext_databits = DATA_8;
        break;
    default:
        qext_databits = DATA_8;
        break;
    }

    return qext_databits;
}

ParityType lpc_flash_app::convert_parity_to_qextserial(QString *par){
    ParityType qext_parity;

    if(*par == "None"){
        qext_parity = PAR_NONE;
    }
    else if(*par == "Odd"){
        qext_parity = PAR_ODD;
    }
    else if(*par == "Even"){
        qext_parity = PAR_EVEN;
    }
    else if(*par == "Mark"){
#ifdef WINDOWS
        qext_parity = PAR_MARK;
#endif
#ifdef LINUX
        qext_parity = PAR_NONE;
#endif

    }
    else if(*par == "Space"){
        qext_parity = PAR_SPACE;
    }
    else{
        qext_parity = PAR_NONE;
    }

    return qext_parity;
}

StopBitsType lpc_flash_app::convert_stopbits_to_qextserial(float stopbits_num){
    StopBitsType qext_stopbits;

    if(stopbits_num == 1){
        qext_stopbits = STOP_1;
    }
    else if(stopbits_num == 1.5){
#ifdef WINDOWS
        qext_stopbits = STOP_1_5;
#endif
#ifdef LINUX
        qext_stopbits = STOP_1;
#endif
    }
    else if(stopbits_num == 2){
        qext_stopbits = STOP_2;
    }
    else{
        qext_stopbits = STOP_1;
    }

    return qext_stopbits;
}
