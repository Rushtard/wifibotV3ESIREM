#-------------------------------------------------
#
# Project created by QtCreator 2015-02-09T14:30:07
#
#-------------------------------------------------

QT       += core gui network
QT	 +=widgets
QT	 +=webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Wifibot
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    connexion.cpp \
    communication.cpp \
    aide.cpp

HEADERS  += mainwindow.h \
    connexion.h \
    communication.h \
    aide.h

FORMS    += mainwindow.ui \
        connexion.ui \
    aide.ui

RESOURCES += \
    img/res.qrc

OTHER_FILES +=


