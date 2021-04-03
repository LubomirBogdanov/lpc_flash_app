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
#ifndef GUI_FILEIO_H
#define GUI_FILEIO_H

#include <QString>
#include <QFile>
#include <QString>
#include <QTextStream>
#include "errors.h"

#define MAINCFG "../lpc_flash_app/configs/main.cfg"

class gui_fileio{
public:
    gui_fileio();
    ~gui_fileio();
    bool read_entire_file(QString *filename, QStringList *contents);
    bool read_entire_file(QString *filename, QByteArray *contents);
    bool read_entire_config_file(QString *filename, QStringList *contents);
    bool write_entire_file(QString *filename, QStringList *contents);
    bool write_entire_binary_file(QString *filename, QByteArray *contents);
};

#endif // GUI_FILEIO_H
