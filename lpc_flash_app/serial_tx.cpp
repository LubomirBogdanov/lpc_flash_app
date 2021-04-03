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
#include <iostream>
#include <iomanip>
using namespace std;

tx_thread::tx_thread(){
    /*QByteArray input_msg("M`\"``$'D!``#Q`0``^0$``````````````````)W:_^\\````````````````!");
    //QByteArray input_msg("M`\\\"`");
    //QByteArray input_msg("$%`^H%P``");
    //QByteArray input_msg("#P``");
    QByteArray output_msg;
    decode_UU(&input_msg, &output_msg);
    display_byte_array("output", output_msg);
    validate_checksum(&output_msg, 1531);*/

    bin_contents_received.clear();
    bin_contents_received_size = 0;
    download_stage = 1;
}

void tx_thread::set_stage(unsigned long stg){
    download_stage = stg;
}

void tx_thread::set_target_name(QString *mcu){
   supplied_target_name = *mcu;
   supplied_target_id = get_part_id(&supplied_target_name);
}

void tx_thread::set_binary_data(QByteArray *bin_file_data){
    bin_contents = *bin_file_data;
    bin_contents_size = bin_contents.size();    
}

void tx_thread::set_verify(bool vrfy){
    verify = vrfy;
}

void tx_thread::set_xtal_freq(QString *freq){
    xtal_freq = *freq;
}

unsigned long tx_thread::get_part_id(QString *mcu){
    if(*mcu == "lpc1315"){
        return 0x3a010523;
    }
    else if(*mcu == "lpc1316"){
        return 0x1a018524;
    }
    else if(*mcu == "lpc1317"){
        return 0x1a020525;
    }
    else if(*mcu == "lpc1345"){
        return 0x28010541;
    }
    else if(*mcu == "lpc1346"){
        return 0x08018542;
    }
    else if(*mcu == "lpc1347"){
        return 0x08020543;
    }

    return 0;
}

QString tx_thread::get_part_id_name(unsigned long target_id){
    QString target_name;

    switch(target_id){
    case 0x3a010523:
        target_name = "lpc1315";
        break;
    case 0x1a018524:
        target_name = "lpc1316";
        break;
    case 0x1a020525:
        target_name = "lpc1317";
        break;
    case 0x28010541:
        target_name = "lpc1345";
        break;
    case 0x08018542:
        target_name = "lpc1346";
        break;
    case 0x08020543:
        target_name = "lpc1347";
        break;
    default:
        target_name = "unknown";
        break;
    }

    return target_name;
}

void tx_thread::run(){    
    qDebug()<<"(lpcflashapp) (tx_thread) Starting...";
    if(!bin_contents.isEmpty()){        
        qDebug()<<"(lpcflashapp) (tx_thread) Downloading ...";
        tx_buff.clear();
        on_change_download_stage(255, NULL);
    }
    else{
        errors(BINARY_FILE_EMPTY, NULL);
    }
}

void tx_thread::on_change_download_stage(unsigned long cmd_return_code, QByteArray rx_data){    
    static unsigned long num_of_sectors_to_erase = 0;
    static unsigned long current_sector_write = 0;
    static bool reading_flash = 0;
    static long bytes_downloaded_so_far = 0;
    static unsigned long bytes_in_sram_sector = 0;
    static bool writing_sector_to_sram = 0;    

    QByteArray tx_data;
    QString tmp;
    bool ok;
    bool err = 1;
    unsigned long mcu_id;    

    //qDebug()<<"cmd_return_code = "<<cmd_return_code;
    //qDebug() << "(lpcflashapp) (tx_thread) [*]" << rx_data;
    //qDebug()<<"download_stage = "<<download_stage;

    if((cmd_return_code == CMD_SUCCESS) && (writing_sector_to_sram == 1)){
        //qDebug()<<"goto_stage 9";
        download_stage = 9;
    }
    else if((cmd_return_code == CMD_SUCCESS) || (cmd_return_code == INVALID_COMMAND) || (reading_flash == 1)){
        //qDebug()<<"stage_increment";
        download_stage++;
    }

    if(rx_data == "RESEND"){
        //qDebug()<<"goto_stage 11";
        download_stage = 11;
    }

    switch(download_stage){
    case 1:        
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 1";
        tx_buff.resize(3);
        tx_buff = "?\r\n";
        break;
    case 2:
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 2";
        tx_buff.resize(12);
        tx_buff = "Synchronized";
        break;
    case 3:
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 3";        
        long str_size;
        str_size = xtal_freq.size();
        tx_buff.resize(str_size);
        tx_buff = xtal_freq.toLocal8Bit();
        break;
    case 4:
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 4";
        tx_buff.resize(3);
        tx_buff = "A 0";
        break;
    case 5:
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 5";
        tx_buff.resize(1);
        tx_buff = "J";
        break;
    case 6:
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 6";
        tmp = rx_data;        
        mcu_id = tmp.toInt(&ok, 10);
        if(ok){
            if(supplied_target_id == mcu_id){
                QString found_target = get_part_id_name(supplied_target_id);
                qDebug()<<"(lpcflashapp) (tx_thread) target "<<found_target<<"found!";
                qDebug()<<"(lpcflashapp) (tx_thread) unlocking target!";

                tx_buff.resize(7);
                tx_buff = "U 23130";
            }
            else{
                errors(TARGET_NOT_FOUND, &supplied_target_name);
            }
        }
        else{
            QString tmp_str("on_change_download_stage");
            errors(CONVERSION_FAILED, &tmp_str);
        }
        break;
    case 7:
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 7";

        num_of_sectors_to_erase = ((bin_contents_size/FLASH_SECTOR_SIZE)-1);
        tx_buff = "P 0 ";
        tx_buff.append(QString::number(num_of_sectors_to_erase));
        qDebug()<<"(lpcflashapp) (tx_thread) Preparing sectors: 0 -"<<num_of_sectors_to_erase;
        break;
    case 8:
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 8";
        num_of_sectors_to_erase = ((bin_contents_size/FLASH_SECTOR_SIZE)-1);
        //qDebug()<<"num_of_sectors_to_erase = "<<num_of_sectors_to_erase;
        tx_buff = "E 0 ";
        tx_buff.append(QString::number(num_of_sectors_to_erase));
        qDebug()<<"(lpcflashapp) (tx_thread) Erasing sectors: 0 -"<<num_of_sectors_to_erase;
        break;
    case 9:
wr_p:
        //qDebug()<<"(lpcflashapp) (tx_thread) STAGE 9";
        if(bytes_downloaded_so_far >= bin_contents_size){
            download_stage = 12; //Go to download stage 13
            tx_buff = "inv";
            qDebug()<<"(lpcflashapp) (tx_thread) Sending invalid command!";
            writing_sector_to_sram = 0;
        }
        else{
            writing_sector_to_sram = 0;
            tx_buff = "W ";
            tx_buff.append(QString::number(SRAM_START_ADDR+bytes_in_sram_sector) + " ");
            tx_buff.append(QString::number(TX_BUFF));
            //qDebug()<<"(lpcflashapp) (tx_thread) Writing Host -> SRAM";
            //qDebug()<<"bytes_in_sram_sector = "<<bytes_in_sram_sector;
        }
        break;
    case 10:
        //qDebug()<<"(lpcflashapp) (tx_thread) STAGE 10";
        writing_sector_to_sram = 1;
        tx_data.resize(MAX_UU_DATA);
        tx_data = bin_contents.mid(bytes_downloaded_so_far, MAX_UU_DATA);
        tx_buff = synthesize_UU_line(&tx_data);

        bytes_in_sram_sector += MAX_UU_DATA;
        bytes_downloaded_so_far += MAX_UU_DATA;

        //qDebug()<<"bytes_in_sram_sector = "<<bytes_in_sram_sector;
        //qDebug()<<"bytes_downloaded_so_far"<<bytes_downloaded_so_far;

        if(bytes_in_sram_sector >= FLASH_SECTOR_SIZE){
            //It's time to flash contents of SRAM to flash.
            download_stage = 11; //Go to download stage 12
            bytes_in_sram_sector = 0;
            writing_sector_to_sram = 0;
        }
        break;
    case 11:
        //Checksum acknowledged?
        //qDebug()<<"(lpcflashapp) (tx_thread) STAGE 11";
        if(cmd_return_code == CMD_SUCCESS){
            goto wr_p; //Got any better idea?
        }
        else{
            QString tmp("on_change_download_stage");
            errors(CHECKSUM_FAIL, &tmp);
        }
        break;
    case 12:
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 12";
        if(current_sector_write <= num_of_sectors_to_erase){
            qDebug()<<"(lpcflashapp) (tx_thread) Preparing sectors: "<<current_sector_write<<" - "<<current_sector_write;
            tx_buff = "P ";
            tx_buff.append(QString::number(current_sector_write) + " ");
            tx_buff.append(QString::number(current_sector_write));
        }
        break;
    case 13:
        //Write SRAM to FLASH
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 13";
        if(current_sector_write <= num_of_sectors_to_erase){
            qDebug()<<"(lpcflashapp) (tx_thread) Writing SRAM -> FLASH";
            qDebug()<<"(lpcflashapp) (tx_thread) writing to sector "<<current_sector_write;
            tx_buff = "C ";
            tx_buff.append(QString::number(sector_to_flash_address(current_sector_write)) + " ");
            tx_buff.append(QString::number(SRAM_START_ADDR) + " ");
            tx_buff.append(QString::number(FLASH_SECTOR_SIZE));
            download_stage = 8; //Go to download stage 9

            current_sector_write++;
            //qDebug()<<"[WRITE]: "<<tx_buff;
        }
        break;
    case 14:
        //Download finished!
        qDebug()<<"(lpcflashapp) (tx_thread) STAGE 14";
        qDebug()<<"(lpcflashapp) (tx_thread) Flash write complete!";
        err = 0;        
        num_of_sectors_to_erase = 0;
        current_sector_write = 0;
        reading_flash = 0;
        bytes_downloaded_so_far = 0;
        bytes_in_sram_sector = 0;
        writing_sector_to_sram = 0;
        bin_contents_received.clear();
        download_stage = 1;

        emit flash_write_complete();
        break;

    default:
        err = 0;
        QString temp = QString::number(download_stage);
        errors(WRONG_STAGE, &temp);
        break;
    }

    if(err){        
        tx_buff.append("\r\n");
        //qDebug()<<"[**] "<<tx_buff;
        emit send_serial_data(&tx_buff);
    }
}

void tx_thread::on_set_num_of_sectors_to_upload(unsigned long num_of_sectrs){
    num_of_sectors_to_upload = num_of_sectrs;
}

void tx_thread::on_upload_flash(unsigned long cmd_return_code, QByteArray rx_data){
    static bool reading = 0;
    static QByteArray bincontents_one_sector;
    static unsigned long decoded_bytes = 0;
    static unsigned long decoded_bytes_total = 0;
    static unsigned long current_address_read = 0;
    static unsigned long current_sector_read = 0;

    QByteArray one_uu_line;
    QByteArray one_uu_line_decoded;    
    QString tmp;
    bool ok;
    bool err = 1;
    bool end_of_sector = 0;
    unsigned long mcu_id;
    bool go_to_next_sector = 0;

    //qDebug() << "(lpcflashapp) (tx_thread) [*]" << rx_data;

    if(cmd_return_code == 255){
        download_stage = 1;
    }
    else if((cmd_return_code == CMD_SUCCESS)){        
        download_stage++;        
    }

    switch(download_stage){
    case 1:
        qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 1";
        tx_buff.resize(3);
        tx_buff = "?\r\n";
        break;
    case 2:
        qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 2";
        tx_buff.resize(12);
        tx_buff = "Synchronized";
        break;
    case 3:
        qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 3";
        long str_size;
        str_size = xtal_freq.size();
        tx_buff.resize(str_size);
        tx_buff = xtal_freq.toLocal8Bit();
        break;
    case 4:
        qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 4";
        tx_buff.resize(3);
        tx_buff = "A 0";
        break;
    case 5:
        qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 5";
        tx_buff.resize(1);
        tx_buff = "J";
        break;
    case 6:
        qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 6";
        tmp = rx_data;
        mcu_id = tmp.toInt(&ok, 10);
        if(ok){
            if(supplied_target_id == mcu_id){
                QString found_target = get_part_id_name(supplied_target_id);
                qDebug()<<"(lpcflashapp) (tx_thread) target "<<found_target<<"found!";
                qDebug()<<"(lpcflashapp) (tx_thread) unlocking target!";

                tx_buff.resize(7);
                tx_buff = "U 23130";
            }
            else{
                errors(TARGET_NOT_FOUND, &supplied_target_name);
            }
        }
        else{
            QString tmp_str("on_upload_flash case6");
            errors(CONVERSION_FAILED, &tmp_str);
        }
        break;
    case 7:
rs7:
        go_to_next_sector = 0;
        qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 7";

        if(current_sector_read > (num_of_sectors_to_upload-1)){
            goto rs9;
        }
        else{
            emit upload_operation(1);
            tx_buff = "R ";
            tx_buff.append(QString::number(current_address_read) + " ");
            tx_buff.append(QString::number(FLASH_SECTOR_SIZE));
            err = 1;
        }
        break;
    case 8:
        //qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 8";
        download_stage = 7;
        reading = 1;
        rx_data.append("\r\n");
        //qDebug()<<"recv num = "<<rx_data.size();
        //qDebug()<<"=============================================";
        //qDebug()<<"=============================================";
        //qDebug()<<"=============================================";

        for(int i = 0, j = 0; i < rx_data.size(); i++){            
            if(rx_data[i] == '\r'){
                //qDebug()<<"i = "<<i<<" j = "<<j<<" ((i -1) - j)) = "<<((i - 1) - j);
                one_uu_line = rx_data.mid(j, (i - j));
                qDebug()<<"(lpcflashapp) (tx_thread)****** "<<one_uu_line<<" "<<one_uu_line.size();

                quint16 bytes_in_line = decode_UU(&one_uu_line, &one_uu_line_decoded);
                decoded_bytes += bytes_in_line;
                decoded_bytes_total += bytes_in_line;
                //qDebug()<<"decoded_bytes ="<<decoded_bytes;
                //qDebug()<<"decoded_bytes_total ="<<decoded_bytes_total;

                if(decoded_bytes_total > FLASH_SECTOR_SIZE){
                    end_of_sector = 1;
                }

                if((decoded_bytes > (UPLOAD_BYTES_PER_LINE*UPLOAD_UU_LINES + 1)) || (end_of_sector)){

                    decoded_bytes -= bytes_in_line;
                    decoded_bytes_total -= bytes_in_line;

                    if(end_of_sector){
                        QString tmp(one_uu_line);
                        tmp.toInt(&ok, 10);
                        if(ok){
                            int checksum = tmp.toInt(&ok, 10);
                            //qDebug()<<"-----checksum2: "<<checksum;

                            if(validate_checksum(&bincontents_one_sector, checksum)){
                                bin_contents_received.append(bincontents_one_sector);
                                bincontents_one_sector.clear();
                                bin_contents_received_size = bin_contents_received.size();
                                one_uu_line_decoded.clear();
                                current_address_read += FLASH_SECTOR_SIZE;
                                decoded_bytes_total = 0;
                                decoded_bytes = 0;
                                end_of_sector = 0;

                                download_stage = 7;
                                reading = 0;
                                tx_buff = "OK";

                                current_sector_read++;

                                go_to_next_sector = 1;

                                //hex_dump_byte_array(bin_contents_received);
                                qDebug()<<"(lpcflashapp) (tx_thread) checksum OK ";
                                qDebug()<<"(lpcflashapp) (tx_thread) Read sector complete!";
                            }
                            else{
                                QString tmp_str("on_upload_flash");
                                errors(CHECKSUM_FAIL, &tmp_str);
                                reading = 0;
                                decoded_bytes -= bytes_in_line;
                                decoded_bytes_total -= bytes_in_line;
                                tx_buff = "RESEND";
                            }
                        }
                        else{
                            QString tmp_str("on_upload_flash");
                            errors(CHECKSUM_FAIL, &tmp_str);
                            reading = 0;
                            decoded_bytes -= bytes_in_line;
                            decoded_bytes_total -= bytes_in_line;
                            tx_buff = "RESEND";
                        }
                    }
                    else{
                        QString tmp(one_uu_line);
                        tmp.toInt(&ok, 10);
                        if(ok){
                            int checksum = tmp.toInt(&ok, 10);
                            //qDebug()<<"-----checksum: "<<checksum;

                            if(validate_checksum(&bincontents_one_sector, checksum)){
                                qDebug()<<"(lpcflashapp) (tx_thread) checksum OK ";
                                bin_contents_received.append(bincontents_one_sector);
                                bincontents_one_sector.clear();
                                bin_contents_received_size = bin_contents_received.size();
                                //Acknowledge checksum to the target
                                tx_buff = "OK";
                                reading = 0;
                                decoded_bytes = 0;
                            }
                            else{
                                QString tmp_str("on_upload_flash");
                                errors(CHECKSUM_FAIL, &tmp_str);
                                reading = 0;
                                decoded_bytes -= bytes_in_line;
                                decoded_bytes_total -= bytes_in_line;
                                tx_buff = "RESEND";
                            }
                        }
                        else{
                            QString tmp_str("on_upload_flash READ STAGE 8");
                            errors(CONVERSION_FAILED, &tmp_str);
                        }
                    }
                }
                else{
                    bincontents_one_sector.append(one_uu_line_decoded);
                    one_uu_line_decoded.clear();
                }

                i+=2; //skip '\n'
                j = i;
            }
        }
        //qDebug()<<"=============================================";
        //qDebug()<<"=============================================";
        //qDebug()<<"=============================================";
        err = 1;
        break;
    case 9:
rs9:    qDebug()<<"(lpcflashapp) (tx_thread) READ STAGE 9";
        qDebug()<<"(lpcflashapp) (tx_thread) upload finished!";

        err = 0;
        download_stage = 1;

        reading = 0;
        bincontents_one_sector.clear();
        decoded_bytes = 0;
        decoded_bytes_total = 0;
        current_address_read = 0;
        current_sector_read = 0;

        emit upload_operation(0);
        emit upload_complete(bin_contents_received);
        break;

    default:
        err = 0;
        QString temp = QString::number(download_stage);
        errors(WRONG_STAGE, &temp);
        break;
    }    

    if(err && (!reading)){
        tx_buff.append("\r\n");
        //qDebug()<<"tx_buff = "<<tx_buff;
        emit send_serial_data(&tx_buff);

        if(go_to_next_sector){            
            goto rs7; //LPC does not respond in any way after the checksum acknowledge at the last byte in sector.
        }
    }
}

void tx_thread::on_erase_entire_flash(unsigned long cmd_return_code, QByteArray rx_data){
    QString tmp;
    bool ok;
    bool err = 1;
    quint8 start_sector;
    unsigned long mcu_id;
    static unsigned long num_of_sectors_to_erase = 0;

    //qDebug() << "(lpcflashapp) (tx_thread) [*]" << rx_data;

    if(cmd_return_code == 255){
        download_stage = 1;
    }
    else if((cmd_return_code == CMD_SUCCESS)){
        download_stage++;
    }

    switch(download_stage){
    case 1:
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 1";
        tx_buff.resize(3);
        tx_buff = "?\r\n";
        break;
    case 2:
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 2";
        tx_buff.resize(12);
        tx_buff = "Synchronized";
        break;
    case 3:
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 3";
        long str_size;
        str_size = xtal_freq.size();
        tx_buff.resize(str_size);
        tx_buff = xtal_freq.toLocal8Bit();
        break;
    case 4:
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 4";
        tx_buff.resize(3);
        tx_buff = "A 0";
        break;
    case 5:
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 5";
        tx_buff.resize(1);
        tx_buff = "J";
        break;
    case 6:
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 6";
        tmp = rx_data;
        mcu_id = tmp.toInt(&ok, 10);
        if(ok){
            if(supplied_target_id == mcu_id){
                QString found_target = get_part_id_name(supplied_target_id);
                qDebug()<<"(lpcflashapp) (tx_thread) target "<<found_target<<"found!";
                qDebug()<<"(lpcflashapp) (tx_thread) unlocking target!";

                tx_buff.resize(7);
                tx_buff = "U 23130";
            }
            else{
                errors(TARGET_NOT_FOUND, &supplied_target_name);
            }
        }
        else{
            QString tmp_str("on_upload_flash case6");
            errors(CONVERSION_FAILED, &tmp_str);
        }
        break;
    case 7:
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 7";

        num_of_sectors_to_erase = (NUM_OF_SECTORS-1);
        tx_buff = "P 0 ";
        tx_buff.append(QString::number(num_of_sectors_to_erase));
        qDebug()<<"(lpcflashapp) (tx_thread) Preparing sectors: 0 -"<<num_of_sectors_to_erase;
        break;
    case 8:
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 8";
        num_of_sectors_to_erase = (NUM_OF_SECTORS-1);
        tx_buff = "E 0 ";
        tx_buff.append(QString::number(num_of_sectors_to_erase));
        qDebug()<<"(lpcflashapp) (tx_thread) Erasing sectors: 0 -"<<num_of_sectors_to_erase;
        break;
    case 9:
        start_sector = 1;
        qDebug()<<"(lpcflashapp) (tx_thread) ERASE STAGE 9";
        qDebug()<<"(lpcflashapp) (tx_thread) Blank checking: "<<start_sector<<" - "<<num_of_sectors_to_erase;
        tx_buff = "I 1 ";
        tx_buff.append(QString::number(num_of_sectors_to_erase));
        break;
    case 10:
        if(cmd_return_code == CMD_SUCCESS){
            qDebug()<<"(lpcflashapp) (tx_thread) Device erased!";
            err = 0;
            num_of_sectors_to_erase = 0;
            emit erase_complete();
        }
        else{
            QString tmp("on_erase_entire_flash");
            errors(FLASH_ERASE_FAILED, &tmp);
        }
        break;

    default:
        err = 0;
        QString temp = QString::number(download_stage);
        errors(WRONG_STAGE, &temp);
        break;
    }

    if(err){
        tx_buff.append("\r\n");
        //qDebug()<<"tx_buff = "<<tx_buff;
        emit send_serial_data(&tx_buff);
    }
}

void tx_thread::on_reset_target(){
    qDebug()<<"(lpcflashapp) (tx_thread) Resetting target...";
    tx_buff.resize(3);
    tx_buff = "G 0 T\r\n";
    emit send_serial_data(&tx_buff);
}

/*!
 * \brief tx_thread::sector_to_flash_address - get flash sector start address.
 * \param flash_sector - the flash sector of interest.
 * \return The start address of the flash sector.
 */
unsigned long tx_thread::sector_to_flash_address(unsigned long flash_sector){
    unsigned long flash_start;
    switch(flash_sector){
    case 0:
        flash_start = 0x00000000;
        break;
    case 1:
        flash_start = 0x00001000;
        break;
    case 2:
        flash_start = 0x00002000;
        break;
    case 3:
        flash_start = 0x00003000;
        break;
    case 4:
        flash_start = 0x00004000;
        break;
    case 5:
        flash_start = 0x00005000;
        break;
    case 6:
        flash_start = 0x00006000;
        break;
    case 7:
        flash_start = 0x00007000;
        break;
    case 8:
        flash_start = 0x00008000;
        break;
    case 9:
        flash_start = 0x00009000;
        break;
    case 10:
        flash_start = 0x0000A000;
        break;
    case 11:
        flash_start = 0x0000B000;
        break;
    case 12:
        flash_start = 0x0000C000;
        break;
    case 13:
        flash_start = 0x0000D000;
        break;
    case 14:
        flash_start = 0x0000E000;
        break;
    case 15:
        flash_start = 0x0000F000;
        break;
    default:
        flash_start = 0x00000000;
        break;
    }

    return flash_start;
}


