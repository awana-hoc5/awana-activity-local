#-------------------------------------------------
#
# Project created by QtCreator 2014-08-11T15:07:22
#
#-------------------------------------------------

QT       += core gui sql  network testlib
#QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#INCLUDEPATH += ../qextserialport_google/src
#LIBS += -L../qextserialport_google/build -lqextserialport1

TARGET = csv_gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    signindialog.cpp

HEADERS  += mainwindow.h \
    signindialog.h

FORMS    += mainwindow.ui \
    signindialog.ui
