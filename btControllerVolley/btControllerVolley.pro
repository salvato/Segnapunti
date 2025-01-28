#Copyright (C) 2023  Gabriele Salvato

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
    ../CommonFiles/button.cpp \
    ../CommonFiles/edit.cpp \
    ../CommonFiles/btscorecontroller.cpp \
    ../CommonFiles/utility.cpp \
    generalsetuparguments.cpp \
    generalsetupdialog.cpp \
    main.cpp \
    volleyapplication.cpp \
    volleycontroller.cpp

HEADERS += \
    ../CommonFiles/btclient.h \
    ../CommonFiles/btscorecontroller.h \
    ../CommonFiles/button.h \
    ../CommonFiles/edit.h \
    ../CommonFiles/panelorientation.h \
    ../CommonFiles/utility.h \
    generalsetuparguments.h \
    generalsetupdialog.h \
    volleyapplication.h \
    volleycontroller.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/drawable-xhdpi/icon.png \
    android/res/drawable-xxhdpi/icon.png \
    android/res/drawable-xxxhdpi/icon.png \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml \
    ButtonIcons/ExchangeVolleyField.png \
    ButtonIcons/GeneralSetup.png \
    ButtonIcons/Minus.png \
    ButtonIcons/New-Game-Volley.png \
    ButtonIcons/New-Set-Volley.png \
    ButtonIcons/PanelSetup.png \
    ButtonIcons/PlaySlides.png \
    ButtonIcons/PlaySpots.png \
    ButtonIcons/Plus.png \
    ButtonIcons/sign_stop.png \
    Logo_SSD_UniMe.png \
    Logo_UniMe.png \
    ball0.png \
    ball1.png \
    myLogo.ico \
    myLogo.png

RESOURCES += \
    Resources.qrc


contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_MIN_SDK_VERSION = 23
}


contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android

    ANDROID_MIN_SDK_VERSION = 23
}
