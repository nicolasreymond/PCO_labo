CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

SOURCES += \
    ../../src/pcoconditionvariable.cpp \
    ../../src/pcohoaremonitor.cpp \
    ../../src/pcologger.cpp \
    ../../src/pcomanager.cpp \
    ../../src/pcomutex.cpp \
    ../../src/pcosemaphore.cpp \
    ../../src/pcothread.cpp

HEADERS += \
    ../../src/pcoconditionvariable.h \
    ../../src/pcohoaremonitor.h \
    ../../src/pcologger.h \
    ../../src/pcomanager.h \
    ../../src/pcomutex.h \
    ../../src/pcosemaphore.h \
    ../../src/pcothread.h

# Default rules for deployment.
unix {
    includes.path = /usr/local/include/pcosynchro
    includes.files = ../../src/*.h
    INSTALLS    += includes


    target.path = /usr/local/lib
    INSTALLS    += target
}
