#-------------------------------------------------
#
# Project created by QtCreator 2015-05-15T14:49:42
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = ChatServer
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++11

TEMPLATE = app
QMAKE_CXXFLAGS += -g -fno-omit-frame-pointer #-fsanitize=address,undefined
#QMAKE_LFLAGS += -fsanitize=address,undefined

SOURCES += main.cpp \
    tcp/tcpsocket.cpp \
    tcp/tcpserversocket.cpp \
    http/httpobject.cpp \
    http/httprequest.cpp \
    http/httpresponce.cpp \
    http/httputils.cpp \
    http/httpserver.cpp \
    chat/chatserver.cpp \
    logger.cpp \
    executor.cpp \
    http/httpmatcher.cpp \
    fd_closer.cpp \
    preparation.cpp

HEADERS += \
    logger.h \
    tcp/tcpsocket.h \
    tcp/tcpserversocket.h \
    http/httpobject.h \
    http/httprequest.h \
    http/httpresponse.h \
    http/httputils.h \
    http/httpserver.h \
    chat/chatserver.h \
    executor.h \
    http/httpmatcher.h \
    preparation.h \
    fd_closer.h
