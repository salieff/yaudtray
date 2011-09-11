QT        += core gui

TARGET     = yaudtray
TEMPLATE   = app

CONFIG    += debug qdbus

SOURCES += \
    src/main.cpp \
    src/yaudtrayapp.cpp \
    src/devinfo.cpp \
    src/devinfowidget.cpp

HEADERS  += \
    src/yaudtrayapp.h \
    src/devinfo.h \
    src/devinfowidget.h

RESOURCES += \
    main.qrc

FORMS += \
    src/devinfowidget.ui
