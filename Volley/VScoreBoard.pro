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
    timeoutwindow.cpp \
    volleyapplication.cpp \
    volleycontroller.cpp \
    volleypanel.cpp

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
    timeoutwindow.h \
    volleyapplication.h \
    volleycontroller.h \
    volleypanel.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    VScoreBoard.qrc

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
../CommonFiles/ShadersRPi4/fAngular.glsl \
../CommonFiles/ShadersRPi4/fBookFlip.glsl \
../CommonFiles/ShadersRPi4/fBounce.glsl \
../CommonFiles/ShadersRPi4/fCircleopen.glsl \
../CommonFiles/ShadersRPi4/fCrosshatch.glsl \
../CommonFiles/ShadersRPi4/fCrosswarp.glsl \
../CommonFiles/ShadersRPi4/fDisplacement.glsl \
../CommonFiles/ShadersRPi4/fDoomScreen.glsl \
../CommonFiles/ShadersRPi4/fDreamy.glsl \
../CommonFiles/ShadersRPi4/fFade.glsl \
../CommonFiles/ShadersRPi4/fFilmBurn.glsl \
../CommonFiles/ShadersRPi4/fFlyEye.glsl \
../CommonFiles/ShadersRPi4/fInvertedPageCurl.glsl \
../CommonFiles/ShadersRPi4/fMorph.glsl \
../CommonFiles/ShadersRPi4/fMultiply_blend.glsl \
../CommonFiles/ShadersRPi4/fPerlin.glsl \
../CommonFiles/ShadersRPi4/fPinwheel.glsl \
../CommonFiles/ShadersRPi4/fPixelize.glsl \
../CommonFiles/ShadersRPi4/fPolkaDotsCurtain.glsl \
../CommonFiles/ShadersRPi4/fPowerKaleido.glsl \
../CommonFiles/ShadersRPi4/fRadial.glsl \
../CommonFiles/ShadersRPi4/fRipple.glsl \
../CommonFiles/ShadersRPi4/fSwap.glsl \
../CommonFiles/ShadersRPi4/fSwirl.glsl \
../CommonFiles/ShadersRPi4/fWaterDrop.glsl \
../CommonFiles/ShadersRPi4/fWind.glsl \
../CommonFiles/ShadersRPi4/vShader.glsl \
.gitignore \

windows:RC_ICONS += ../CommonFiles/Loghi/Logo.ico
