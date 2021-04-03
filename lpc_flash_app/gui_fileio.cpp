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
#include "gui_fileio.h"

gui_fileio::gui_fileio(){

}

gui_fileio::~gui_fileio(){

}

bool gui_fileio::read_entire_file(QString *filename, QByteArray *contents){
    bool err = 0;

    QFile file(*filename);
    err = file.open(QIODevice::ReadOnly);
    if(err){
        err = 1;
        *contents = file.readAll();        
        file.close();
    }
    else{
        errors(CANT_OPEN_FILE, filename);
    }

    return err;
}

/*!
 * \brief gui_fileio::read_entire_file - Reads a file and stores its contents
 * in the QStringList buffer. Returns one on success and zero if the opening
 * of the file failed.
 *
 * \param filename - the name of the file to be opened.
 * \param contents - a buffer containing all the lines in the file.
 *
 * \return 0 - the operation was successful; 1 - it failed.
 */
bool gui_fileio::read_entire_file(QString *filename, QStringList *contents){
    bool err = 0;
    QString currentline;

    QFile file(*filename);
    err = file.open(QIODevice::ReadOnly|QIODevice::Text);
    if(err){
        err = 1;
        QTextStream FileStream(&file);
        while(!FileStream.atEnd()){
            //Do not use *Contents << FileStream.readLine(); because it will remove the newlines from the file.
            currentline = FileStream.readLine();
            *contents << currentline;
        }
        file.close();
    }
    else{
        errors(CANT_OPEN_FILE, filename);
    }

    return err;
}

/*!
 * \brief gui_fileio::read_entire_config_file - Same as read_entire_file( ) but
 * excludes comments, denoted with '#'.
 *
 * \param filename - the name of the configuration file to be opened.
 * \param contents - a buffer containing all the lines in the configuration file.
 * \return 0 - the operation was successful; 1 - it failed.
 */
bool gui_fileio::read_entire_config_file(QString *filename, QStringList *contents){
    bool err = 0;
    QString currentline;

    QFile file(*filename);
    err = file.open(QIODevice::ReadOnly|QIODevice::Text);
    if(err){
        err = 1;
        QTextStream FileStream(&file);
        while(!FileStream.atEnd()){
            //Do not use *Contents << FileStream.readLine(); because it will remove the newlines from the file.
            currentline = FileStream.readLine();
            if(currentline.contains("#")){ //Exclude comments
                int ind = currentline.indexOf("#");
                ind = currentline.size() - ind;
                currentline.chop(ind);
            }

            *contents << currentline;

        }
        file.close();
    }
    else{
        errors(CANT_OPEN_FILE, filename);
    }

    return err;
}

/*!
 * \brief gui_fileio::write_entire_file - Writes the QStringList buffer to
 * a file. Returns one on success and zero if the writing to the file failed.
 *
 * \param filename - the name of the file to write to.
 * \param contents - a buffer containing all the lines to be written to the file.
 *
 * \return 0 - the operation was successful; 1 - it failed.
 */
bool gui_fileio::write_entire_file(QString *filename, QStringList *contents){
    bool err = 0;

    QFile file(*filename);
    err = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if(err){
        err = 1;
        QTextStream FileStream(&file);
        for(qint32 i = 0; i < contents->size(); i++){
            FileStream << contents->at(i);
            FileStream << endl;
        }

        file.close();
    }
    else{
       errors(CANT_WRITE_FILE, filename);
    }

    return err;
}

bool gui_fileio::write_entire_binary_file(QString *filename, QByteArray *contents){
    bool err = 0;

    QFile file(*filename);
    err = file.open(QIODevice::WriteOnly);
    if(err){
        err = 1;
        file.write(*contents);
        file.close();
    }
    else{
       errors(CANT_WRITE_FILE, filename);
    }

    return err;
}
