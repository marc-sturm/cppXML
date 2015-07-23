#c++11 and c++14 support
CONFIG += c++14

#base settings
QT       -= gui
QT       += gui xml xmlpatterns
TEMPLATE = lib
TARGET = cppXML
DEFINES += CPPXML_LIBRARY

#enable O3 optimization
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

#copy DLL to bin folder
DESTDIR = ../../bin/

#include cppCORE library
INCLUDEPATH += $$PWD/../cppCORE
LIBS += -L$$PWD/../../bin -lcppCORE

HEADERS += XMLHelper.h \

SOURCES += XMLHelper.cpp \

