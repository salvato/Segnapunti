/*
 *
Copyright (C) 2023  Gabriele Salvato

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

#include "qwidget.h"
#include "remainingtimedialog.h"
#include "../CommonFiles/scorecontroller.h"
#include "../CommonFiles/panelorientation.h"

#include <QTimer>
#include <QElapsedTimer>
#ifndef __ARM_ARCH
#include <QSerialPortInfo>
#include <QSerialPort>
#else
#include <gpiod.h>
#endif

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(Edit)
QT_FORWARD_DECLARE_CLASS(myLabel)
QT_FORWARD_DECLARE_CLASS(Button)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(WaterPoloPanel)
QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(QPushButton)


class WaterPoloCtrl : public ScoreController
{
    Q_OBJECT

public:
    WaterPoloCtrl(QFile *myLogFile, QWidget *parent = nullptr);
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
#ifndef Q_OS_ANDROID
    bool          connectToAlarm();
#endif

private slots:
    void onAppStart();
    void onGameTimeChanging();
    void onTimeUpdate();
    void closeEvent(QCloseEvent*);
    void onTimeOutIncrement(int iTeam);
    void onTimeOutDecrement(int iTeam);
    void onCountStart(int iTeam);
    void onCountStop(int iTeam);
    void onScoreIncrement(int iTeam);
    void onScoreDecrement(int iTeam);
    void onTeamTextChanged(QString sText, int iTeam);
    void onTeamTextEditDone();
    void onButtonChangeFieldClicked();
    void onButtonNewPeriodClicked();
    void onButtonNewGameClicked();
    void onChangePanelOrientation(PanelOrientation orientation);
    void onStopAlarm();

private:
    void          buildControls();
    void          setEventHandlers();
    void          sendAll();
    void          btSendAll();
    void          processBtMessage(QString sMessage);
    void          exchangeField();
    void          startNewPeriod();
    void          disableUi();
    void          enableUi();
    void          changeFocus();

private:
    WaterPoloPanel* pWaterPoloPanel;
    RemainingTimeDialog* pRemainingTimeDialog;
    int             iTimeout[2]{};
    int             iScore[2]{};
    int             iPeriod;
    Edit*           pTeamName[2]{};
    Edit*           pTimeoutEdit[2]{};
    Edit*           pScoreEdit[2]{};
    Edit*           pPeriodEdit{};
    Edit*           pTimeEdit{};
    Button*         pTimeoutIncrement[2]{};
    Button*         pTimeoutDecrement[2]{};
    Button*         pScoreIncrement[2]{};
    Button*         pScoreDecrement[2]{};
    Button*         pCountStart{};
    Button*         pCountStop{};
    QLabel*         pTimeoutLabel{};
    QLabel*         pScoreLabel{};
    QLabel*         pPeriodLabel{};

    QPushButton*    pNewPeriodButton{};
    QPushButton*    pNewGameButton{};
    QPushButton*    pChangeFieldButton{};

    bool            bFontBuilt;
    QPalette        panelPalette;
    QLinearGradient panelGradient;
    QBrush          panelBrush;
    int             maxTeamNameLen;
    QTimer          startTimer;
    QTimer          updateTimer;
    QElapsedTimer   tempoTimer;
    qint64          runMilliSeconds;
    qint64          remainingMilliSeconds;
    int             lastM;
    int             lastS;
    bool            isAlarmFound;
#ifndef Q_OS_ANDROID
#ifndef __ARM_ARCH
    QByteArray             requestData;/*!< The string sent to the Arduino */

    enum commands {
        ENQUIRY        = char(0x05),
        ACK            = char(0x06),
        BELL           = char(0x07),
        CANCEL         = char(0x18)
    };

    QSerialPort::BaudRate  baudRate;
    QSerialPort            serialPort;
    QSerialPortInfo        serialPortinfo;
    QList<QSerialPortInfo> serialPorts;
    int                    currentPort;
    int                    waitTimeout;
    QByteArray             responseData;
#else
    const char* chipname = "gpiochip0";
    struct gpiod_chip*     pChip;
    struct gpiod_line*     pLineAlarm;
    int                    alarmIO;
#endif
    QTimer                 alarmDurationTimer;
    int                    alarmDuration;
#endif
};

