include("../lib.pri")

#base settings
QT       -= gui
QT       += xml
TARGET = cppXML
DEFINES += CPPXML_LIBRARY

#include cppCORE library
INCLUDEPATH += $$PWD/../cppCORE
LIBS += -L$$PWD/../../bin -lcppCORE

#include libxml2
win32: INCLUDEPATH += $$PWD/../../libxml2/include/
win32: LIBS += -L$$PWD/../../libxml2/libs/ -lxml2
unix: INCLUDEPATH += $$system(pkg-config --cflags libxml-2.0)
unix: !macx: QMAKE_CXXFLAGS += $$system(pkg-config --cflags libxml-2.0)
unix: LIBS += -lxml2

HEADERS += \
    XMLHelper.h

SOURCES += \
    XMLHelper.cpp
