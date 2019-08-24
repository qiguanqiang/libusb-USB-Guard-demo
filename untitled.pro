#-------------------------------------------------
#
# Project created by QtCreator 2019-08-01T14:47:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = untitled
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    disabler.cpp \
    main_functions.cpp

HEADERS  += mainwindow.h \
    libs.h \
    disabler.h \
    main_functions.h

FORMS    +=

unix: LIBS += -lusb-1.0

DISTFILES +=
