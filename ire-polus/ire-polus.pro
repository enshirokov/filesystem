#-------------------------------------------------
#
# Project created by QtCreator 2017-09-21T22:13:09
#
#-------------------------------------------------

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ire-polus
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    createfiledialog.cpp

HEADERS  += mainwindow.h \
    createfiledialog.h

RESOURCES += \
    images.qrc

