#-------------------------------------------------
#
# Project created by QtCreator 2015-04-28T20:21:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport serialport

TARGET = FreeScope
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    serialreader.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    serialreader.h

FORMS    += mainwindow.ui
