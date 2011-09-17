#***************************************************************************
#*   Copyright (C) 2011 by Alexander S. Salieff                            *
#*   salieff@mail.ru                                                       *
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#*   This program is distributed in the hope that it will be useful,       *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU General Public License for more details.                          *
#*                                                                         *
#*   You should have received a copy of the GNU General Public License     *
#*   along with this program; if not, write to the                         *
#*   Free Software Foundation, Inc.,                                       *
#*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
#***************************************************************************

QT        += core gui

TARGET     = yaudtray
TEMPLATE   = app

#CONFIG    += debug qdbus
CONFIG    += release qdbus

SOURCES += \
    src/main.cpp \
    src/yaudtrayapp.cpp \
    src/devinfo.cpp \
    src/devinfowidget.cpp \
    src/helpers.cpp

HEADERS  += \
    src/yaudtrayapp.h \
    src/devinfo.h \
    src/devinfowidget.h \
    src/helpers.h

RESOURCES += \
    main.qrc

FORMS += \
    src/devinfowidget.ui
