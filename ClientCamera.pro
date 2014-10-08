TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11

SOURCES += src/main.cpp \
    src/camera.cpp

LIBS += `pkg-config opencv --libs`

HEADERS += \
    src/camera.h
