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
QT += opengl
QT += openglwidgets
QT += bluetooth

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../CommonFiles/btserver.cpp \
    ../CommonFiles/button.cpp \
    ../CommonFiles/edit.cpp \
    ../CommonFiles/scorecontroller.cpp \
    ../CommonFiles/scorepanel.cpp \
    ../CommonFiles/slidewidget.cpp \
    ../CommonFiles/utility.cpp \
    generalsetuparguments.cpp \
    generalsetupdialog.cpp \
    main.cpp \
    remainingtimedialog.cpp \
    waterpoloapp.cpp \
    waterpoloctrl.cpp \
    waterpolopanel.cpp

HEADERS += \
    ../CommonFiles/btserver.h \
    ../CommonFiles/button.h \
    ../CommonFiles/edit.h \
    ../CommonFiles/panelorientation.h \
    ../CommonFiles/scorecontroller.h \
    ../CommonFiles/scorepanel.h \
    ../CommonFiles/slidewidget.h \
    ../CommonFiles/utility.h \
    generalsetuparguments.h \
    generalsetupdialog.h \
    remainingtimedialog.h \
    waterpoloapp.h \
    waterpoloctrl.h \
    waterpolopanel.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    WaterPolo.qrc

DISTFILES += \
    ../CommonFiles/ButtonIcons/GeneralSetup.png \
    ../CommonFiles/ButtonIcons/Minus.png \
    ../CommonFiles/ButtonIcons/PanelSetup.png \
    ../CommonFiles/ButtonIcons/PlaySlides.png \
    ../CommonFiles/ButtonIcons/PlaySpots.png \
    ../CommonFiles/ButtonIcons/Plus.png \
    ../CommonFiles/ButtonIcons/sign_stop.png \
    ../CommonFiles/ButtonIcons/video-display.png \
    ../CommonFiles/Shaders/fAngular.glsl \
    ../CommonFiles/Shaders/fBookFlip.glsl \
    ../CommonFiles/Shaders/fBounce.glsl \
    ../CommonFiles/Shaders/fCircleopen.glsl \
    ../CommonFiles/Shaders/fCrosshatch.glsl \
    ../CommonFiles/Shaders/fCrosswarp.glsl \
    ../CommonFiles/Shaders/fDisplacement.glsl \
    ../CommonFiles/Shaders/fDoomScreen.glsl \
    ../CommonFiles/Shaders/fDreamy.glsl \
    ../CommonFiles/Shaders/fFade.glsl \
    ../CommonFiles/Shaders/fFilmBurn.glsl \
    ../CommonFiles/Shaders/fFlyEye.glsl \
    ../CommonFiles/Shaders/fInvertedPageCurl.glsl \
    ../CommonFiles/Shaders/fMorph.glsl \
    ../CommonFiles/Shaders/fMultiply_blend.glsl \
    ../CommonFiles/Shaders/fPerlin.glsl \
    ../CommonFiles/Shaders/fPinwheel.glsl \
    ../CommonFiles/Shaders/fPixelize.glsl \
    ../CommonFiles/Shaders/fPolkaDotsCurtain.glsl \
    ../CommonFiles/Shaders/fPowerKaleido.glsl \
    ../CommonFiles/Shaders/fRadial.glsl \
    ../CommonFiles/Shaders/fRipple.glsl \
    ../CommonFiles/Shaders/fSwap.glsl \
    ../CommonFiles/Shaders/fSwirl.glsl \
    ../CommonFiles/Shaders/fWaterDrop.glsl \
    ../CommonFiles/Shaders/fWind.glsl \
    ../CommonFiles/Shaders/vShader.glsl \
    .gitignore \
    ButtonIcons/ExchangeVolleyField.png \
    ButtonIcons/Go.png \
    ButtonIcons/New-Game-Volley.png \
    ButtonIcons/New-Set-Volley.png \
    ButtonIcons/PlaySpots.png \
    ButtonIcons/StopTime.png \
    Images/Cartella.png \
    Images/Controller.png \
    Images/Eseguibile.png \
    Images/Exchange.png \
    Images/Go.png \
    Images/Minus.png \
    Images/NewGame.png \
    Images/NewPeriod.png \
    Images/NewTime.png \
    Images/Plus.png \
    Images/Setup.png \
    Images/SetupDialog.png \
    Images/Slide.png \
    Images/Spot.png \
    Images/Stop.png \
    Images/Tabellone.png \
    LICENSE \
    Logo.ico \
    README.md \
    Waterpolo.odt \
    Waterpolo.pdf

win32:RC_ICONS += Logo.ico
