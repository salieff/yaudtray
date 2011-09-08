QT        += core

TARGET     = yaudtray
TEMPLATE   = app

CONFIG    += qdbus

SOURCES += \
    src/main.cpp \
    src/yaudtrayapp.cpp \
    src/devinfo.cpp

HEADERS  += \
    src/yaudtrayapp.h \
    src/devinfo.h
