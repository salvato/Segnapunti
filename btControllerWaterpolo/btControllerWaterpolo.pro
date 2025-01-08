#Copyright (C) 2025  Gabriele Salvato

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT += core
QT += gui
QT += widgets
QT += bluetooth


CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../CommonFiles/btclient.cpp \
    ../CommonFiles/btscorecontroller.cpp \
    ../CommonFiles/button.cpp \
    ../CommonFiles/edit.cpp \
    ../CommonFiles/utility.cpp \
    generalsetuparguments.cpp \
    generalsetupdialog.cpp \
    main.cpp \
    remainingtimedialog.cpp \
    waterpoloapplication.cpp \
    waterpolocontroller.cpp

HEADERS += \
    ../CommonFiles/btclient.h \
    ../CommonFiles/btscorecontroller.h \
    ../CommonFiles/button.h \
    ../CommonFiles/edit.h \
    ../CommonFiles/utility.h \
    generalsetuparguments.h \
    generalsetupdialog.h \
    remainingtimedialog.h \
    waterpoloapplication.h \
    waterpolocontroller.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources.qrc
