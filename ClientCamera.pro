TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11
QMAKE_MAKEFILE += build/makefile

SOURCES += src/main.cpp \
    src/camera.cpp \
    src/silhouette.cpp

LIBS += `pkg-config opencv --libs`

HEADERS += \
    src/camera.h \
    src/silhouette.h
