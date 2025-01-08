/*
 *
Copyright (C) 2025  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#pragma once

#include "remainingtimedialog.h"
#include "../CommonFiles/btscorecontroller.h"
#include "../CommonFiles/panelorientation.h"


QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(Edit)
QT_FORWARD_DECLARE_CLASS(myLabel)
QT_FORWARD_DECLARE_CLASS(Button)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(QPushButton)


class WaterpoloController : public BtScoreController
{
    Q_OBJECT
public:
    WaterpoloController(QFile *myLogFile, QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event);

protected:
    void          GetSettings();
    void          SaveSettings();
    void          setWindowLayout();
    QGridLayout*  CreateGamePanel();
    QHBoxLayout*  CreateGameButtons();
    void          buildFontSizes();
    void          SaveStatus();
    void          GeneralSetup();
    void          processTextMessage(QString sMessage);

private slots:
    void closeEvent(QCloseEvent*);
    void onGameTimeChanging();
//TODO
    // void onTimeUpdate();
    void onButtonNewPeriodClicked();
    void onTimeOutIncrement(int iTeam);
    void onTimeOutDecrement(int iTeam);
    void onCountStart(int iTeam);
    void onCountStop(int iTeam);
    void onScoreIncrement(int iTeam);
    void onScoreDecrement(int iTeam);
    void onButtonChangeFieldClicked();
    void onChangePanelOrientation(PanelOrientation orientation);

private:
    void          buildControls();
    void          setEventHandlers();
    void          disableUi();
    void          enableUi();
#ifdef Q_OS_ANDROID
    void          keepScreenOn();
#endif
    void startNewPeriod();

private:
    RemainingTimeDialog* pRemainingTimeDialog;
    int           iTimeout[2]{};
    int           iScore[2]{};
    int           iPeriod;
    Edit*         pTeamName[2]{};
    Edit*         pTimeoutEdit[2]{};
    Edit*         pSetsEdit[2]{};
    Edit*         pScoreEdit[2]{};
    Edit*         pPeriodEdit{};
    Edit*         pTimeEdit{};
    Button*       pTimeoutIncrement[2]{};
    Button*       pTimeoutDecrement[2]{};
    Button*       pScoreIncrement[2]{};
    Button*       pScoreDecrement[2]{};
    Button*       pCountStart{};
    Button*       pCountStop{};
    QLabel*       pTimeoutLabel{};
    QLabel*       pScoreLabel{};
    QLabel*       pPeriodLabel{};

    QPushButton*  pNewPeriodButton{};
    QPushButton*  pChangeFieldButton{};

    bool            bFontBuilt;
    QPalette        panelPalette;
    QLinearGradient panelGradient;
    QBrush          panelBrush;
    int             maxTeamNameLen= 15;
    qint64          runMilliSeconds;
    qint64          remainingMilliSeconds;
};

