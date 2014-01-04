#-------------------------------------------------
#
# Project created by QtCreator 2014-01-02T15:19:06
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Code128Generator
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    gencode128.cpp

HEADERS  += mainwindow.h \
    gencode128.h

FORMS    += mainwindow.ui
