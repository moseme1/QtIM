QT       += core gui widgets sql network

CONFIG += c++17
CONFIG += utf8_source

TARGET = QtIM
TEMPLATE = app

# 包含所有头文件
HEADERS += chatwindow.h \
           contactmodel.h \
           datamanager.h \
           globaldefine.h \
           loginwindow.h \
           mainwindow.h \
           messagemodel.h \
           networkmanager.h \
           sqlrepository.h \
           workerthread.h

# 包含所有源文件
SOURCES += chatwindow.cpp \
           contactmodel.cpp \
           datamanager.cpp \
           loginwindow.cpp \
           main.cpp \
           mainwindow.cpp \
           messagemodel.cpp \
           networkmanager.cpp \
           sqlrepository.cpp \
           workerthread.cpp

# 包含所有界面文件
FORMS += chatwindow.ui \
         loginwindow.ui \
         mainwindow.ui

DESTDIR = build