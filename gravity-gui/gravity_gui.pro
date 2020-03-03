#-------------------------------------------------
#
# Project created by QtCreator 2017-07-07T10:04:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gravity_gui
TEMPLATE = app

CONFIG -= debug_and_release
CONFIG += warn_off
CONFIG += debug

LIBS += -lncurses

QMAKE_CXXFLAGS += -Wall -Wno-strict-aliasing

SOURCES += main.cpp\
           gravity_gui.cpp \
           g_gui_impl.cpp \
           g_progs_impl.cpp \
           g_prog.cpp \
           ReplWidget.cpp \ 
    helpbox.cpp

HEADERS  += gravity_gui.h ReplWidget.h g_prog.h \
    helpbox.h

FORMS    += gravity_gui.ui \
    helpbox.ui

#DEFINES += VERSION=\\\"1.2.0\\\"

RESOURCES += \
    gravity_gui.qrc

# If you run qtcreator, it will clobber the autotools
# Makefile.  This causes qmake to output a Makefile
# with this name for development within the 
# qtcreator gui.
MAKEFILE=Makefile.qt

