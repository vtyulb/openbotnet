#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T23:27:18
#
#-------------------------------------------------

QT       -= core

QT       -= gui

TARGET = bot
CONFIG   += console
CONFIG   -= app_bundle

LIBS += -lcryptopp

TEMPLATE = app


SOURCES += main.cpp base64.cpp
HEADERS += base64.h
