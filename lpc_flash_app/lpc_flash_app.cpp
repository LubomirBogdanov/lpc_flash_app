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
#include "ui_lpc_flash_app.h"
#include "rx_tx_threads.h"

lpc_flash_app::lpc_flash_app(QWidget *parent) : QMainWindow(parent), ui(new Ui::lpc_flash_app){
    ui->setupUi(this);

    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());

    init_defaults(&defaults);
    read_config();    
    set_gui_params(&defaults);

    ui->download_button->setEnabled(0);
    ui->erase_button->setEnabled(0);
    ui->disconnect_button->setEnabled(0);
    ui->connect_button->setEnabled(1);
    ui->pushButton_upload->setEnabled(0);
    ui->pushButton_reset->setEnabled(0);

    serial_rx_thread = NULL;
    serial_tx_thread = NULL;
}

lpc_flash_app::~lpc_flash_app(){
    if(serial_rx_thread){
        delete serial_rx_thread;
    }

    if(serial_tx_thread){
        delete serial_tx_thread;
    }

    delete ui;
}

void lpc_flash_app::init_defaults(gui_defaults_t *defs){
    defs->baud = 2400;
#ifdef LINUX
    defs->port = "/dev/ttyS0";
#endif
#ifdef WINDOWS
    defs->port = "COM1";
#endif
    defs->databits = 8;
    defs->mcu = "lpc1343";
    defs->parity = "None";
    defs->stopbits = 1.0;
    defs->binary_file_path = "/home/myuser/mybinary.bin";
    defs->upload_file_path = "/home/myuser/uploadbinary.bin";
    defs->xtal_freq = QString::number(10000);
    defs->add_valid_code_signature = 1;
    defs->verify = 1;
}

void lpc_flash_app::read_config(){
    QString path = MAINCFG;
    QStringList main_cfg;    
    bool ok = 1;
    unsigned long number;
    float stop_bits;

    if(config_file.read_entire_file(&path, &main_cfg)){
        for(long i=0; i < main_cfg.size(); i++){

            if(main_cfg.at(i).contains("PORT")){
                defaults.port = main_cfg.at(i).section('\t', 1, -1);
            }

            if(main_cfg.at(i).contains("MCU")){
                defaults.mcu = main_cfg.at(i).section('\t', 1, -1);
            }

            if(main_cfg.at(i).contains("BAUD")){
                number = main_cfg.at(i).section('\t', 1, -1).toInt(&ok, 10);

                if(ok){
                    defaults.baud = number;
                }
                else{
                    errors(CONVERSION_FAILED, NULL);
                }
            }

            if(main_cfg.at(i).contains("DATABITS")){
                number = main_cfg.at(i).section('\t', 1, -1).toInt(&ok, 10);

                if(ok){
                    defaults.databits = number;
                }
                else{
                    errors(CONVERSION_FAILED, NULL);
                }
            }

            if(main_cfg.at(i).contains("PARITY")){
                defaults.parity = main_cfg.at(i).section('\t', 1, -1);
            }

            if(main_cfg.at(i).contains("STOPBITS")){
                stop_bits = main_cfg.at(i).section('\t', 1, -1).toFloat(&ok);

                if(ok){
                    defaults.stopbits = stop_bits;
                }
                else{
                    errors(CONVERSION_FAILED, NULL);
                }
            }

            if(main_cfg.at(i).contains("VALIDCODE")){
                number = main_cfg.at(i).section('\t', 1, -1).toInt(&ok, 10);

                if(ok){
                    defaults.add_valid_code_signature = number;
                }
                else{
                    errors(CONVERSION_FAILED, NULL);
                }
            }

            if(main_cfg.at(i).contains("VERIFY")){
                number = main_cfg.at(i).section('\t', 1, -1).toInt(&ok, 10);

                if(ok){
                    defaults.verify = number;
                }
                else{
                    errors(CONVERSION_FAILED, NULL);
                }
            }

            if(main_cfg.at(i).contains("BINARYFILE")){
                defaults.binary_file_path = main_cfg.at(i).section('\t', 1, -1);
            }

            if(main_cfg.at(i).contains("UPLOADFILE")){
                defaults.upload_file_path = main_cfg.at(i).section('\t', 1, -1);
            }

            if(main_cfg.at(i).contains("XTALFREQ")){
                defaults.xtal_freq = main_cfg.at(i).section('\t', 1, -1);
            }
        }
    }

    qDebug()<<"(lpcflashapp) Default MCU: "<<defaults.mcu;
    qDebug()<<"(lpcflashapp) Default BAUD: "<<defaults.baud;
    qDebug()<<"(lpcflashapp) Default DATABITS: "<<defaults.databits;
    qDebug()<<"(lpcflashapp) Default PARITY: "<<defaults.parity;
    qDebug()<<"(lpcflashapp) Default STOPBITS: "<<defaults.stopbits;
    qDebug()<<"(lpcflashapp) Default BINARYFILE: "<<defaults.binary_file_path;
    qDebug()<<"(lpcflashapp) Default VALIDCODE: "<<defaults.add_valid_code_signature;
    qDebug()<<"(lpcflashapp) Default VERIFY: "<<defaults.verify;
}

void lpc_flash_app::set_gui_params(gui_defaults_t *params){
    ui->lineEdit_port_name->setText(params->port);
    ui->lineEdit_file_path->setText(params->binary_file_path);
    ui->lineEdit_file_path_upload->setText(params->upload_file_path);
    ui->comboBox_baud->setCurrentIndex(convert_baud_to_index(params->baud));
    ui->comboBox_data_bits->setCurrentIndex(convert_databits_to_index(params->databits));
    ui->comboBox_mcu->setCurrentIndex(convert_mcu_to_index(&params->mcu));
    ui->comboBox_parity->setCurrentIndex(convert_parity_to_index(&params->parity));
    ui->comboBox_stop_bits->setCurrentIndex(convert_stopbits_to_index(params->stopbits));
    ui->lineEdit_xtal_freq->setText(params->xtal_freq);
    ui->checkBox_valid_code->setChecked(params->add_valid_code_signature);
    ui->checkBox_verify->setChecked(params->verify);    
}

void lpc_flash_app::on_connect_button_pressed(){
    PortSettings serial_settings;

    BaudRateType qext_baud = convert_baud_to_qextserial(defaults.baud);
    ParityType qext_par = convert_parity_to_qextserial(&defaults.parity);
    DataBitsType qext_databits = convert_databits_to_qextserial(defaults.databits);
    StopBitsType qext_stopbits = convert_stopbits_to_qextserial(defaults.stopbits);

    serial_settings.BaudRate = qext_baud;
    serial_settings.Parity = qext_par;
    serial_settings.DataBits = qext_databits;
    serial_settings.StopBits = qext_stopbits;
    serial_settings.Timeout_Millisec = 10;
    serial_settings.FlowControl = FLOW_OFF;


    if(!serial_rx_thread){
        serial_rx_thread = new rx_thread;
    }

    if(!serial_tx_thread){
        serial_tx_thread = new tx_thread;
    }

    connect(serial_rx_thread, SIGNAL(port_is_open()), this, SLOT(on_port_is_open()));
    connect(this, SIGNAL(disconnect_button_pressed()), serial_rx_thread, SLOT(on_disconnect()));
    connect(serial_tx_thread, SIGNAL(send_serial_data(QByteArray *)), serial_rx_thread, SLOT(on_send_serial_data(QByteArray *)));
    connect(this, SIGNAL(reset_target()), serial_tx_thread, SLOT(on_reset_target()));
    connect(serial_rx_thread, SIGNAL(change_download_stage(unsigned long , QByteArray)), serial_tx_thread, SLOT(on_change_download_stage(unsigned long , QByteArray)));   
    connect(serial_tx_thread, SIGNAL(flash_write_complete()), this, SLOT(on_flash_write_complete()));

    serial_rx_thread->open_port(&defaults.port, &serial_settings);
    QString temp = ui->lineEdit_xtal_freq->text();
    serial_rx_thread->set_xtal_freq(&temp);
    serial_tx_thread->set_xtal_freq(&temp);
    serial_tx_thread->set_target_name(&defaults.mcu);
}

void lpc_flash_app::on_port_is_open(){    
    ui->download_button->setEnabled(1);
    ui->erase_button->setEnabled(1);
    ui->disconnect_button->setEnabled(1);
    ui->pushButton_upload->setEnabled(1);
    ui->pushButton_reset->setEnabled(1);
    ui->connect_button->setEnabled(0);
}

void lpc_flash_app::on_actionExit_triggered(){
    close();
}

void lpc_flash_app::on_download_button_pressed(){
    if(bin_contents.isEmpty()){
        QString str = ui->lineEdit_file_path->text();
        read_binary(&str);
    }
    serial_rx_thread->start();
    serial_tx_thread->set_binary_data(&bin_contents);
    serial_tx_thread->set_verify(defaults.verify);
    serial_tx_thread->start();
}

void lpc_flash_app::on_flash_write_complete(){
    if(defaults.verify){
        disconnect(serial_rx_thread, SIGNAL(change_download_stage(unsigned long, QByteArray)), 0, 0);
        connect(serial_rx_thread, SIGNAL(change_download_stage(ulong,QByteArray)), serial_tx_thread, SLOT(on_upload_flash(ulong,QByteArray)));
        connect(serial_tx_thread, SIGNAL(upload_complete(QByteArray)), this, SLOT(on_upload_complete(QByteArray)));
        connect(this, SIGNAL(upload_flash(ulong,QByteArray)), serial_tx_thread, SLOT(on_upload_flash(ulong,QByteArray)));
        connect(serial_tx_thread, SIGNAL(upload_operation(bool)), serial_rx_thread, SLOT(on_upload_operation(bool)));
        connect(this, SIGNAL(set_num_of_sectors_to_upload(unsigned long)), serial_tx_thread, SLOT(on_set_num_of_sectors_to_upload(unsigned long)));

        connect(serial_tx_thread, SIGNAL(upload_complete(QByteArray)), this, SLOT(on_verify_binaries(QByteArray)));

        unsigned long used_sectors = bin_contents.size() / FLASH_SECTOR_SIZE;
        //qDebug()<<"used_sectors ="<<used_sectors;

        serial_tx_thread->download_stage = 3;

        emit set_num_of_sectors_to_upload(used_sectors);
        emit upload_flash(CMD_SUCCESS, NULL);
    }
}

void lpc_flash_app::on_verify_binaries(QByteArray uploaded_contents){
    bool err = 0;

    disconnect(serial_tx_thread, SIGNAL(upload_complete(QByteArray)), this, SLOT(on_verify_binaries(QByteArray)));

    //qDebug()<<"================================WRITE====================================";
    //hex_dump_byte_array(bin_contents);
    //qDebug()<<"=========================================================================";

    qDebug()<<"================================READ=====================================";
    hex_dump_byte_array(uploaded_contents);
    qDebug()<<"=========================================================================";

    for(long i = 0; i < bin_contents.size(); i++){
        if(bin_contents.at(i) != uploaded_contents.at(i)){
            err = 1;
            QString tmp;
            tmp = "written[" + QString::number(i) + "] = " + QString::number(bin_contents.at(i));
            tmp += " <=> ";
            tmp += "read[" + QString::number(i) + "] = " + QString::number(uploaded_contents.at(i));
            errors(VERIFY_FAIL, &tmp);
        }
    }

    if(err){
        QString tmp("on_verify_binaries");
        errors(FLASH_WRITE_FAILED, &tmp);
    }
    else{
        qDebug()<<"(lpcflashapp) Verify passed!";
    }
}

void lpc_flash_app::on_disconnect_button_pressed(){
    emit disconnect_button_pressed();

    ui->disconnect_button->setEnabled(0);
    ui->pushButton_reset->setEnabled(0);
    ui->pushButton_upload->setEnabled(0);
    ui->download_button->setEnabled(0);
    ui->erase_button->setEnabled(0);
    ui->connect_button->setEnabled(1);

    serial_tx_thread->set_stage(0);
    serial_tx_thread->quit();
    serial_rx_thread->quit();
}

void lpc_flash_app::on_browse_file_pressed(){    
    QString new_binary_path;
    QFileDialog open_file_dialog;

    new_binary_path = open_file_dialog.getOpenFileName(this, tr("Open binary file"), defaults.binary_file_path, tr(" (*.bin)"));

    read_binary(&new_binary_path);
}

void lpc_flash_app::read_binary(QString *new_binary_path){
    quint32 lpc_word = 0;
    quint32 valid_code_check_sum = 0;

    QString temp_str;
    if(!new_binary_path->isEmpty()){
        qDebug()<<"(lpcflashapp) Binary file path: "<<*new_binary_path;
        defaults.binary_file_path = *new_binary_path;
        ui->lineEdit_file_path->setText(defaults.binary_file_path);

        temp_str = defaults.binary_file_path;

        if(binary_file.read_entire_file(&temp_str, &bin_contents)){            
            hex_dump_byte_array(bin_contents);
        }

        qDebug()<<"(lpcflashapp) Done reading file! File size: "<<bin_contents.size()<<" bytes";
    }

    if(defaults.add_valid_code_signature){
        qDebug()<<"(lpcflashapp) Adding valid code signature...";

        for(long i = 0; i < 24; i+=4){

            const quint8 *data = (const quint8*)bin_contents.constData();

            /*if((i % 16) == 0){
                cout<<endl<<setfill('0')<<setw(8)<<hex<<i<<" ";
            }
            printf("%02x", data[i+3]);
            printf("%02x", data[i+2]);
            printf("%02x", data[i+1]);
            printf("%02x ", data[i]);*/

            lpc_word = (data[i+3]<<24) | (data[i+2]<<16) | (data[i+1]<<8) | data[i];
            valid_code_check_sum += lpc_word;            
        }
        //fflush(stdout);
        //qDebug()<<" ";

        //qDebug()<<"(lpcflashapp) Check sum of words 0 - 6: "<<valid_code_check_sum;
        valid_code_check_sum = ~valid_code_check_sum;
        //qDebug()<<"(lpcflashapp) Check sum inverted: "<<valid_code_check_sum;
        valid_code_check_sum += 1;
        //qDebug()<<"(lpcflashapp) Check sum 1 added: "<<QString::number(valid_code_check_sum);

        bin_contents[0x1f] = valid_code_check_sum>>24;
        bin_contents[0x1e] = (valid_code_check_sum>>16) & 0xFF;
        bin_contents[0x1d] = (valid_code_check_sum>>8) & 0xFF;
        bin_contents[0x1c] = (valid_code_check_sum) & 0xFF;

        const quint8 *data = (const quint8*)bin_contents.constData();
        printf("(lpcflashapp) VALID CODE: 0x%02x",  data[0x1f]);
        printf("%02x", data[0x1e]);
        printf("%02x", data[0x1d] );
        printf("%02x ", data[0x1c] );
        fflush(stdout);
        qDebug()<<" ";        
    }

    int num_of_sectors;
    int sector_remaining_bytes;
    num_of_sectors = bin_contents.size() / FLASH_SECTOR_SIZE;
    if(num_of_sectors < NUM_OF_SECTORS){
        num_of_sectors++;
        sector_remaining_bytes = (num_of_sectors*FLASH_SECTOR_SIZE) - bin_contents.size();
        //qDebug()<<"bin_contents.size() ="<<bin_contents.size();
        //qDebug()<<"num_of_sectors = "<<num_of_sectors;        
        qDebug()<<"(lpcflashapp) Filling up sector with "<<sector_remaining_bytes<<" remaining bytes.";        
        for(int i = 0; i < sector_remaining_bytes; i++){
            bin_contents.append(0xff);
        }
    }
    else{
        qDebug()<<"(lpcflashapp) No need of filling up sector.";
    }

    qDebug()<<"Bytes to write: "<<bin_contents.size();
}

void lpc_flash_app::on_pushButton_reset_pressed(){
    emit reset_target();
}

void lpc_flash_app::on_checkBox_valid_code_clicked(bool checked){
    if(checked){
        defaults.add_valid_code_signature = 1;
    }
    else{
        defaults.add_valid_code_signature = 0;
    }
}

void lpc_flash_app::on_checkBox_verify_clicked(bool checked){
    if(checked){
        defaults.verify = 1;
    }
    else{
        defaults.verify = 0;
    }
}

void lpc_flash_app::on_pushButton_upload_pressed(){
    disconnect(serial_rx_thread, SIGNAL(change_download_stage(unsigned long, QByteArray)), 0, 0);
    connect(serial_rx_thread, SIGNAL(change_download_stage(ulong,QByteArray)), serial_tx_thread, SLOT(on_upload_flash(ulong,QByteArray)));
    connect(serial_tx_thread, SIGNAL(upload_complete(QByteArray)), this, SLOT(on_upload_complete(QByteArray)));
    connect(this, SIGNAL(upload_flash(ulong,QByteArray)), serial_tx_thread, SLOT(on_upload_flash(ulong,QByteArray)));
    connect(serial_tx_thread, SIGNAL(upload_operation(bool)), serial_rx_thread, SLOT(on_upload_operation(bool)));
    connect(this, SIGNAL(set_num_of_sectors_to_upload(unsigned long)), serial_tx_thread, SLOT(on_set_num_of_sectors_to_upload(unsigned long)));

    connect(serial_tx_thread, SIGNAL(upload_complete(QByteArray)), this, SLOT(on_write_uploaded_binary(QByteArray)));

    emit set_num_of_sectors_to_upload(NUM_OF_SECTORS);
    emit upload_flash(255, NULL);
}

void lpc_flash_app::on_write_uploaded_binary(QByteArray bin_data){
    qDebug()<<"================================ALLREAD==================================";
    hex_dump_byte_array(bin_data);
    qDebug()<<"=========================================================================";

    if(binary_file.write_entire_binary_file(&defaults.upload_file_path, &bin_data)){
        qDebug()<<"(lpc_flash_app) MCU contents uploaded to file: "<<defaults.upload_file_path<<"!";
    }

    disconnect(serial_tx_thread, SIGNAL(upload_complete(QByteArray)), this, SLOT(on_write_uploaded_binary(QByteArray)));
}

void lpc_flash_app::on_upload_complete(QByteArray bin_data){
    //qDebug()<<"upload complete slot!";
    disconnect(serial_rx_thread, SIGNAL(change_download_stage(ulong,QByteArray)), 0, 0);
    connect(serial_rx_thread, SIGNAL(change_download_stage(unsigned long , QByteArray)), serial_tx_thread, SLOT(on_change_download_stage(unsigned long , QByteArray)));

    //hex_dump_byte_array(bin_data);

    uploaded_bin_contents = bin_data;
}

void lpc_flash_app::on_erase_button_pressed(){
    disconnect(serial_rx_thread, SIGNAL(change_download_stage(unsigned long, QByteArray)), 0, 0);
    connect(serial_rx_thread, SIGNAL(change_download_stage(ulong,QByteArray)), serial_tx_thread, SLOT(on_erase_entire_flash(ulong,QByteArray)));
    connect(serial_tx_thread, SIGNAL(erase_complete()), this, SLOT(on_erase_complete()));
    connect(this, SIGNAL(erase_flash(ulong,QByteArray)), serial_tx_thread, SLOT(on_erase_entire_flash(ulong,QByteArray)));

    emit erase_flash(255, NULL);
}

void lpc_flash_app::on_erase_complete(void){

}

void lpc_flash_app::on_actionAbout_triggered(){
    QMessageBox msgBox;
    QString tmp;
    tmp = "LPC Flash Tool v" + QString::number(VERSION);
    tmp += "\nAuthor: Lubomir Bogdanov, lbogdanov@tu-sofia.bg";
    msgBox.setText(tmp);
    msgBox.exec();
}

void lpc_flash_app::on_actionInfo_triggered(){
    QMessageBox msgBox;
    QString tmp;
    tmp = "Copyright (C) 2021 Lubomir Bogdanov\n" \
    "Contributor Lubomir Bogdanov <lbogdanov@tu-sofia.bg>\n\n" \
    "This file is part of lpc_flash_app.\n" \
    "lpc_flash_app is free software: you can redistribute it and/or modify " \
    "it under the terms of the GNU Lesser General Public License as published by " \
    "the Free Software Foundation, either version 3 of the License, or: " \
    "(at your option) any later version. " \
    "lpc_flash_app is distributed in the hope that it will be useful, " \
    "but WITHOUT ANY WARRANTY; without even the implied warranty of " \
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the " \
    "GNU Lesser General Public License for more details. " \
    "You should have received a copy of the GNU Lesser General Public License " \
    "along with lpc_flash_app.  If not, see <http://www.gnu.org/licenses/>. ";
    msgBox.setText(tmp);
    msgBox.exec();
}
