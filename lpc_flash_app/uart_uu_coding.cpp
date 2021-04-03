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
#include "uart_uu_coding.h"

void hex_dump_byte_array(QByteArray byte_arr){
    const quint8 *data = (const quint8*)byte_arr.constData();

    for(long i = 0; i < byte_arr.size(); i+=4){
        if((i % 16) == 0){
            cout<<endl<<setfill('0')<<setw(8)<<hex<<i<<" ";
        }
        printf("%02x", data[i+3]);
        printf("%02x", data[i+2]);
        printf("%02x", data[i+1]);
        printf("%02x ", data[i]);
    }
    fflush(stdout);
    qDebug()<<" ";
}

/*!
 * \brief convert_eight_to_six - takes 3 normal bytes and returns 4 bytes where only the first 6-bit
 * values are meaningful. Used by the UU encoder.
 *
 * \param input - the input 3 bytes
 * \param output - the output 4 bytes
 *
 * \return None.
 */
void convert_eight_to_six(QByteArray *input, QByteArray *output){
    QByteArray temp;

    temp.resize(4);
    temp[3] = (input->at(2))&0x3F;
    temp[2] = ((input->at(1)&0xF)<<2) | ((input->at(2)&0xC0)>>6);
    temp[1] = ((input->at(0)&0x03)<<4) | ((input->at(1)&0xF0)>>4);
    temp[0] = (input->at(0)&0xFC)>>2;

    *output = temp.mid(0);
}

/*!
 * \brief convert_six_add_twenty - the 6-bit values are added with
 * 0x20. If the value is 0x00, it is added with 0x60. Used by the encoder.
 *
 * \param input - must contain 4 6-bit values
 * \param output - must contain 4 6-bit values
 *
 * \return None.
 */
void convert_six_add_twenty(QByteArray *input, QByteArray *output){
    QByteArray temp;
    temp.resize(4);

    for(quint8 i = 0; i < 4; i++){
        if(input->at(i) == 0x00){
            temp[i] = input->at(i) + 0x60;
        }
        else{
            temp[i] = input->at(i) + 0x20;
        }
    }

    *output = temp.mid(0);
}

/*!
 * \brief convert_six_subtract_twenty - the 6-bit values are subtracted with
 * 0x20. If the value is 0x60, it is transformed to 0x00. Used by the decoder.
 *
 * \param input - must contain 4 6-bit values
 * \param output - must contain 4 6-bit values
 *
 * \return None.
 */
void convert_six_subtract_twenty(QByteArray *input, QByteArray *output){
    QByteArray temp;
    temp.resize(4);

    for(quint8 i = 0; i < 4; i++){
        if(input->at(i) == 0x60){
            temp[i] = 0x00;
        }
        else{
            temp[i] = input->at(i) - 0x20;
        }
    }

    *output = temp.mid(0);
}

void display_byte_array(QString name, QByteArray my_arr){
    QString single_line;
    QChar ch1, ch2;
    QString tmp_str;
    QByteArray temp_local_buff;
    temp_local_buff = my_arr.toHex();
    single_line = name;
    single_line += " = {";
    for(int i = 0; i < temp_local_buff.size(); i+=2){
        tmp_str = " 0x";
        ch1 = temp_local_buff.at(i);
        ch2 = temp_local_buff.at(i+1);
        tmp_str += ch1;
        tmp_str += ch2;
        single_line += tmp_str;
        tmp_str.clear();
    }
    single_line += " };";
    qDebug()<<single_line;
}

unsigned long encode_UU(QByteArray *input, QByteArray *output){
    QByteArray three_bytes;
    QByteArray four_bytes;
    QByteArray temp;
    unsigned long remainder = input->size() % 3;
    long num_of_six_bit_values = 0;

    if(remainder != 0){
        //Add padding bytes
        remainder = 3 - remainder;
        //qDebug()<<"remainder = "<<remainder;
        int new_size = input->size() + remainder;

        temp.resize(new_size);
        temp = input->mid(0); //Copy input to temp.
        temp[new_size-1] = 0x00;
        if(remainder == 2){
            temp[new_size-2] = 0x00;
        }

        num_of_six_bit_values = (new_size*8)/6;
    }
    else{
        temp = input->mid(0); //Copy input to temp.
        num_of_six_bit_values = (temp.size()*8)/6;
    }

    //qDebug()<<"num_of_six_bit_values = "<<num_of_six_bit_values;

    three_bytes.resize(3);
    four_bytes.resize(4);
    for(int i = 0; i < temp.size(); i+=3){

        three_bytes[0] = temp[i];
        three_bytes[1] = temp[i+1];
        three_bytes[2] = temp[i+2];

        convert_eight_to_six(&three_bytes, &four_bytes);
        convert_six_add_twenty(&four_bytes, &four_bytes);

        output->append(four_bytes);
    }

    return num_of_six_bit_values;
}

/*!
 * \brief convert_six_to_eight - takes 4 bytes where only the first 6 bits are meaningful and returns 3 normal bytes.
 * Used by the UU decoder.
 *
 * \param input - the input 4 bytes
 * \param output - the output 3 bytes
 *
 * \return None.
 */
void convert_six_to_eight(QByteArray *input, QByteArray *output){
    QByteArray temp;

     temp[0] = (input->at(0)<<2) | (input->at(1)&0x30)>>4;
     temp[1] = ((input->at(1)&0x0F)<<4) | ((input->at(2)&0xFC)>>2);
     temp[2] = ((input->at(2)&03)<<6) | (input->at(3)&0x3F);

    *output = temp.mid(0);
}

unsigned long decode_UU(QByteArray *input, QByteArray *output){
    bool less_than_three = 0;
    quint8 bytes_chop;
    unsigned long decoded_bytes = 0;
    QByteArray three_bytes;
    QByteArray four_bytes;
    QByteArray temp;
    //QByteArray output_temp;

    quint8 num_of_bytes_in_line = 0;

    num_of_bytes_in_line = *input[0];
    num_of_bytes_in_line -= 32;
    //qDebug()<<"num_of_bytes_in_line = "<<num_of_bytes_in_line;

    if(num_of_bytes_in_line < 3){
        less_than_three = 1;
        bytes_chop = 3 - num_of_bytes_in_line;
    }

    //QString test(*input);
    //qDebug()<<"STRING: "<<test<<" STR_SIZE:"<<test.size();
    //display_byte_array("uu_line", *input);


    three_bytes.resize(3);
    four_bytes.resize(4);
    temp = input->mid(1);

    //qDebug()<<"SIZE: "<<temp.size();
    //display_byte_array("temp", temp);

    for(int i = 0; i < temp.size() ; i+=4){

        four_bytes[3] = temp[i+3];   //LSB
        four_bytes[2] = temp[i+2];
        four_bytes[1] = temp[i+1];
        four_bytes[0] = temp[i]; //MSB

        //display_byte_array("four_bytes0-0x20", four_bytes);
        convert_six_subtract_twenty(&four_bytes, &four_bytes);
        //display_byte_array("four_bytes-0x20", four_bytes);
        convert_six_to_eight(&four_bytes, &three_bytes);
        //display_byte_array("three_bytes-8bit", three_bytes);

        if(less_than_three){
            three_bytes.chop(bytes_chop);
        }

        output->append(three_bytes);

        decoded_bytes++;

        //output_temp.append(three_bytes);
    }

    //quint8 output_temp_size = output_temp.size();
    //output->append(output_temp.mid(0, output_temp_size - num_of_bytes_in_line));

    if(less_than_three){
        return num_of_bytes_in_line;
    }
    else{
        return decoded_bytes*3;
    }
}

QByteArray calculate_checksum(QByteArray *input){
    int check_sum = 0;

    for(int i = 0; i < input->size(); i++){
        check_sum += (unsigned char) input->at(i);
    }

    return QString::number(check_sum).toLocal8Bit().append("\r\n");
}

bool validate_checksum(QByteArray *input, unsigned long input_checksum){
    unsigned long calculated_checksum = 0;
    bool ok;

    for(int i = 0; i < input->size(); i++){        
        calculated_checksum += (unsigned char) input->at(i);
    }

    //qDebug()<<"calculated_checksum = "<<calculated_checksum;

    ok = (calculated_checksum == input_checksum) ? 1 : 0;

    return ok;
}

QByteArray synthesize_UU_line(QByteArray *tx_data){
    unsigned long uu_encoded_line_bytes = 0;
    QByteArray transmit_buff;
    QByteArray checksum;
    QByteArray local_buff;

    //display_byte_array("tx_data", *tx_data);

    checksum = calculate_checksum(tx_data);
    local_buff.clear();
    local_buff[0] = tx_data->size()+32;
    uu_encoded_line_bytes = encode_UU(tx_data, &local_buff);

    //display_byte_array("local_buff", local_buff);

    transmit_buff = local_buff.mid(0, uu_encoded_line_bytes+1);

    //qDebug()<<"transmit_buff_size = "<<transmit_buff.size();
    QString tmp = checksum;
    tmp = tmp.section("\r\n", 0, 0);
    qDebug()<<"(lpcflashapp) (uart_uu) "<<transmit_buff<<" "<<tmp;
    //qDebug()<<"checksum = "<<checksum;

    transmit_buff.append("\r\n");
    transmit_buff.append(checksum);

    return transmit_buff;
}
