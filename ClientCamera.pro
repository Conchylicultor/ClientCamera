TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_MAKEFILE += build/makefile
QMAKE_CXXFLAGS += -std=c++11

SOURCES += src/main.cpp \
    src/camera.cpp \
    src/silhouette.cpp

LIBS += `pkg-config opencv --libs`

HEADERS += \
    src/camera.h \
    src/silhouette.h
