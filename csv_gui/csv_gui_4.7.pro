#-------------------------------------------------
#
# Project created by QtCreator 2014-08-11T15:07:22
#
#-------------------------------------------------

QT       += core gui sql  network testlib
#QT       += core gui sql network

INCLUDEPATH += ../qextserialport_google/src
LIBS += -L../qextserialport_google/build -lqextserialport1

TARGET = csv_gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    renderarea.cpp \
    serialinterface.cpp

HEADERS  += mainwindow.h \
    renderarea.h \
    serialinterface.h

FORMS    += mainwindow.ui
