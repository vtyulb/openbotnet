#-------------------------------------------------
#
# Project created by QtCreator 2014-02-25T10:31:31
#
#-------------------------------------------------

QT       += core network

TARGET = server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    server.cpp \
    console.cpp \
    bot.cpp \
    ../bot/base64.cpp

HEADERS += \
    server.h \
    console.h \
    bot.h

LIBS += -lcryptopp

