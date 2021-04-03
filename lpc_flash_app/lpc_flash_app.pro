#
#    Copyright (C) 2021 Lubomir Bogdanov
#    Contributor Lubomir Bogdanov <lbogdanov@tu-sofia.bg>
#    This file is part of lpc_flash_app.
#    lpc_flash_app is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#    lpc_flash_app is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#    You should have received a copy of the GNU Lesser General Public License
#    along with lpc_flash_app.  If not, see <http://www.gnu.org/licenses/>.
#
#-------------------------------------------------
#
# Project created by QtCreator 2016-01-23T21:34:55
#
#-------------------------------------------------

QT       += core gui

include(qextserialport/qextserialport.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lpc_flash_app
TEMPLATE = app

SOURCES += main.cpp\
        lpc_flash_app.cpp \
    gui_fileio.cpp \
    errors.cpp \
    conversion_methods.cpp \
    serial_rx.cpp \
    serial_tx.cpp \
    uart_uu_coding.cpp

HEADERS  += lpc_flash_app.h \
    version.h \
    gui_fileio.h \
    errors.h \
    rx_tx_threads.h \
    uart_uu_coding.h

FORMS    += lpc_flash_app.ui


