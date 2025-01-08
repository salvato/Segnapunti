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

#include <QSettings>
#include <QGridLayout>
#include <QMessageBox>
#include <QResizeEvent>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QScreen>
#include <QDateTime>

#include "generalsetupdialog.h"
#include "../CommonFiles/edit.h"
#include "../CommonFiles/button.h"
#include "../CommonFiles/utility.h"

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniObject>
#endif

#include "waterpolocontroller.h"

////////////////////////////////////////////////////////////////////////////////////
// Il compito del "WaterpoloController" è inviare messaggi al Server del pannello //
// ogni volta che l'utente cambia qualcosa nell'interfaccia.                      //
// Il Server riceve il messaggio, varia ciò che è corretto variare e              //
// invia le nuove informazioni al WaterpoloController affinchè l'interfaccia      //
// rifletta la nuova condizione.                                                  //
////////////////////////////////////////////////////////////////////////////////////


WaterpoloController::WaterpoloController(QFile *myLogFile, QWidget *parent)
    : BtScoreController(myLogFile, parent)
    , pRemainingTimeDialog(new RemainingTimeDialog)
    , bFontBuilt(false)
    , runMilliSeconds(0)
{
    setWindowTitle("Bluetooth Score Controller - © Gabriele Salvato (2025)");
    setWindowIcon(QIcon(":/../CommonFiles/Loghi/Logo.ico"));

    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
    panelGradient.setColorAt(0, QColor(0, 0, START_GRADIENT));
    panelGradient.setColorAt(1, QColor(0, 0, END_GRADIENT));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active,   QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);

    panelPalette.setColor(QPalette::WindowText,      Qt::yellow);
    panelPalette.setColor(QPalette::Base,            Qt::black);
    panelPalette.setColor(QPalette::AlternateBase,   Qt::blue);
    panelPalette.setColor(QPalette::Text,            Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,      Qt::white);
    panelPalette.setColor(QPalette::HighlightedText, Qt::gray);
    panelPalette.setColor(QPalette::Highlight,       Qt::transparent);

    setPalette(panelPalette);

    GetSettings();

    buildControls();
    setWindowLayout();

    setEventHandlers();

#ifdef Q_OS_ANDROID
    keepScreenOn();
#endif
}


void
WaterpoloController::closeEvent(QCloseEvent *event) {
    SaveSettings();
    BtScoreController::closeEvent(event);
    event->accept();
}


#ifdef Q_OS_ANDROID
void
WaterpoloController::keepScreenOn() {
    using namespace QNativeInterface;
    QJniObject activity = QAndroidApplication::context();
    if(activity.isValid()) {
        QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

        if (window.isValid()) {
            const int FLAG_KEEP_SCREEN_ON = 128;
            window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
        }
    }
}
#endif


void
WaterpoloController::resizeEvent(QResizeEvent *event) {
    if(!bFontBuilt) {
        bFontBuilt = true;
        buildFontSizes();
        event->setAccepted(true);
    }
}


void
WaterpoloController::GeneralSetup() {
    GeneralSetupDialog* pGeneralSetupDialog = new GeneralSetupDialog(&gsArgs);
    connect(pGeneralSetupDialog, SIGNAL(changeOrientation(PanelOrientation)),
            this, SLOT(onChangePanelOrientation(PanelOrientation)));
    int iResult = pGeneralSetupDialog->exec();
    if(iResult == QDialog::Accepted) {
        SaveSettings();
    }
    delete pGeneralSetupDialog;
    pGeneralSetupDialog = nullptr;
}


void
WaterpoloController::buildFontSizes() {
    QFont font;
    int iFontSize;
    int hMargin, vMargin;
    QMargins margins;

    font = pTeamName[0]->font();
    font.setCapitalization(QFont::Capitalize);
    margins = pTeamName[0]->contentsMargins();
    vMargin = margins.bottom() + margins.top();
    hMargin = margins.left() + margins.right();
    iFontSize = qMin((pTeamName[0]->width()/pTeamName[0]->maxLength())-2*hMargin,
                     pTeamName[0]->height()-vMargin);

    font.setPixelSize(iFontSize);
    pTeamName[0]->setFont(font);
    pTeamName[1]->setFont(font);
    pTimeoutEdit[0]->setFont(font);
    pTimeoutEdit[1]->setFont(font);

    font.setPixelSize(iFontSize*0.75);
    pTimeoutLabel->setFont(font);
    pScoreLabel->setFont(font);
    pPeriodLabel->setFont(font);
    font.setWeight(QFont::Black);

    font.setPixelSize(2*iFontSize);
    pScoreEdit[0]->setFont(font);
    pScoreEdit[1]->setFont(font);
    pPeriodEdit->setFont(font);
    pTimeEdit->setFont(font);
}


void
WaterpoloController::setWindowLayout() {
    QWidget *widget = new QWidget();
    QGridLayout* pMainLayout = new QGridLayout();

    int gamePanelWidth  = 15;
    int gamePanelHeigth =  8;

    pMainLayout->addLayout(CreateGamePanel(),
                          0,
                          0,
                          gamePanelHeigth,
                          gamePanelWidth);

    pMainLayout->addLayout(CreateGameButtons(),
                          gamePanelHeigth,
                          0,
                          2,
                          8);

    pMainLayout->addLayout(pSpotButtonsLayout,
                          gamePanelHeigth,
                          8,
                          2,
                          gamePanelWidth-5);

    widget->setLayout(pMainLayout);
    setCentralWidget(widget);
    setTabOrder(pTeamName[0], pTeamName[1]);
}


QGridLayout*
WaterpoloController::CreateGamePanel() {
    auto* gamePanel = new QGridLayout();
    // For each Team
    int iRow;
    for(int iTeam=0; iTeam<2; iTeam++) {
        // Matrice X righe e 8 colonne
        iRow = 0;
        // Teams
        gamePanel->addWidget(pTeamName[iTeam], iRow, iTeam*4, 1, 4);
        int iCol = iTeam*5;
        iRow += 1;
        // Score
        gamePanel->addWidget(pScoreDecrement[iTeam], iRow, iCol,   2, 1, Qt::AlignRight);
        gamePanel->addWidget(pScoreEdit[iTeam],      iRow, iCol+1, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(pScoreIncrement[iTeam], iRow, iCol+2, 2, 1, Qt::AlignLeft);
        iRow += 5; // Period & Tempo
        // Timeouts
        gamePanel->addWidget(pTimeoutDecrement[iTeam], iRow, iCol,   1, 1, Qt::AlignRight);
        gamePanel->addWidget(pTimeoutEdit[iTeam],      iRow, iCol+1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(pTimeoutIncrement[iTeam], iRow, iCol+2, 1, 1, Qt::AlignLeft);
    }
    iRow += 1;
    QFrame* myFrame = new QFrame();
    myFrame->setFrameShape(QFrame::HLine);
    gamePanel->addWidget(myFrame, iRow, 0, 1, 10);

    // Labels & Time
    iRow = 1;
    gamePanel->addWidget(pScoreLabel,   iRow, 3, 2, 2);
    iRow += 2;
    gamePanel->addWidget(pPeriodLabel, iRow, 0, 1, 3);

    gamePanel->addWidget(pCountStart, iRow, 4, 3, 1, Qt::AlignRight|Qt::AlignVCenter);
    gamePanel->addWidget(pTimeEdit,   iRow, 5, 3, 2, Qt::AlignVCenter);
    gamePanel->addWidget(pCountStop,  iRow, 7, 3, 1, Qt::AlignLeft|Qt::AlignVCenter);
    iRow += 1;
    //gamePanel->addWidget(pPeriodDecrement, iRow, 0, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
    gamePanel->addWidget(pPeriodEdit,      iRow, 1, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
    //gamePanel->addWidget(pPeriodIncrement, iRow, 2, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
    iRow += 2;
    gamePanel->addWidget(pTimeoutLabel, iRow, 3, 1, 2);

    return gamePanel;
}


QHBoxLayout*
WaterpoloController::CreateGameButtons() {
    auto* gameButtonLayout = new QHBoxLayout();
    QSize iconSize = QSize(48,48);

    QPixmap* pPixmap = new QPixmap(":/ButtonIcons/Exchange.png");
    pChangeFieldButton = new QPushButton(QIcon(*pPixmap), "");
    pChangeFieldButton->setIconSize(iconSize);
    pChangeFieldButton->setFlat(true);
    pChangeFieldButton->setToolTip("Inverti Campo");

    pPixmap->load(":/ButtonIcons/NewPeriod.png");
    pNewPeriodButton  = new QPushButton(*pPixmap, "");
    pNewPeriodButton->setIconSize(iconSize);
    pNewPeriodButton->setFlat(true);
    pNewPeriodButton->setToolTip("Nuovo Periodo");

    delete pPixmap;

    gameButtonLayout->addWidget(pNewPeriodButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pChangeFieldButton);
    gameButtonLayout->addStretch();
    return gameButtonLayout;
}


void
WaterpoloController::GetSettings() {
    gsArgs.maxTimeout           = pSettings->value("waterpolo/maxTimeout", 2).toInt();
    gsArgs.maxPeriods           = pSettings->value("waterpolo/maxPeriods", 4).toInt();
    gsArgs.iTimeDuration        = pSettings->value("waterpolo/TimeDuration", 8).toInt();
    gsArgs.isPanelMirrored      = pSettings->value("panel/orientation",  true).toBool();
    gsArgs.sTeam[0]             = pSettings->value("team1/name", QString(tr("Locali"))).toString();
    gsArgs.sTeam[1]             = pSettings->value("team2/name", QString(tr("Ospiti"))).toString();

    iTimeout[0] = pSettings->value("team1/timeouts", 0).toInt();
    iTimeout[1] = pSettings->value("team2/timeouts", 0).toInt();
    iScore[0]   = pSettings->value("team1/score", 0).toInt();
    iScore[1]   = pSettings->value("team2/score", 0).toInt();
    iPeriod     = pSettings->value("game/period", 1).toInt();

    remainingMilliSeconds = gsArgs.iTimeDuration * 60000;

    // Check Stored Values vs Maximum Values
    for(int i=0; i<2; i++) {
        if(iTimeout[i] > gsArgs.maxTimeout)
            iTimeout[i] = gsArgs.maxTimeout;
    }
    if(iPeriod < 1)
        iPeriod = 1;
    if(iPeriod > gsArgs.maxPeriods)
        iPeriod = gsArgs.maxPeriods;
}


void
WaterpoloController::SaveStatus() {
    // Save Present Game Values
    pSettings->setValue("team1/name", gsArgs.sTeam[0]);
    pSettings->setValue("team2/name", gsArgs.sTeam[1]);
    pSettings->setValue("team1/timeouts", iTimeout[0]);
    pSettings->setValue("team2/timeouts", iTimeout[1]);
    pSettings->setValue("team1/score", iScore[0]);
    pSettings->setValue("team2/score", iScore[1]);
    pSettings->setValue("game/period", iPeriod);
}


void
WaterpoloController::SaveSettings() { // Save General Setup Values
    pSettings->setValue("waterpolo/maxTimeout",  gsArgs.maxTimeout);
    pSettings->setValue("waterpolo/maxPeriods",  gsArgs.maxPeriods);
    pSettings->setValue("waterpolo/TimeDuration", gsArgs.iTimeDuration);
    pSettings->setValue("panel/orientation",      gsArgs.isPanelMirrored);
    pSettings->setValue("team1/name",             pTeamName[0]->text());
    pSettings->setValue("team2/name",             pTeamName[1]->text());
}


void
WaterpoloController::buildControls() {
    QString sString;

    QPixmap plusPixmap, minusPixmap;
    QIcon plusButtonIcon, minusButtonIcon;
    plusPixmap.load(":/CommonFiles/ButtonIcons/Plus.png");
    plusButtonIcon.addPixmap(plusPixmap);
    minusPixmap.load(":/CommonFiles/ButtonIcons/Minus.png");
    minusButtonIcon.addPixmap(minusPixmap);

    QPalette pal = panelPalette;
    pal.setColor(QPalette::Text, Qt::white);

    for(int iTeam=0; iTeam<2; iTeam++){

        // Teams
        pTeamName[iTeam] = new Edit(gsArgs.sTeam[iTeam], iTeam);
        pTeamName[iTeam]->setAlignment(Qt::AlignHCenter);
        pTeamName[iTeam]->setMaxLength(MAX_NAMELENGTH);
        pal.setColor(QPalette::Text, Qt::white);
        pTeamName[iTeam]->setPalette(pal);
        pTeamName[iTeam]->setReadOnly(true);

        // Timeout
        sString = QString("%1").arg(iTimeout[iTeam], 1);
        pTimeoutEdit[iTeam] = new Edit(sString, iTeam);
        pTimeoutEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        pTimeoutEdit[iTeam]->setMaxLength(1);
        pal.setColor(QPalette::Text, Qt::yellow);
        pTimeoutEdit[iTeam]->setPalette(pal);
        pTimeoutEdit[iTeam]->setReadOnly(true);

        // Timeout buttons
        pTimeoutIncrement[iTeam] = new Button("", iTeam);
        pTimeoutIncrement[iTeam]->setIcon(plusButtonIcon);
        pTimeoutIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        pTimeoutDecrement[iTeam] = new Button("", iTeam);
        pTimeoutDecrement[iTeam]->setIcon(minusButtonIcon);
        pTimeoutDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iTimeout[iTeam] == 0)
            pTimeoutDecrement[iTeam]->setEnabled(false);
        if(iTimeout[iTeam] == gsArgs.maxTimeout) {
            pTimeoutIncrement[iTeam]->setEnabled(false);
            pTimeoutEdit[iTeam]->setStyleSheet("background:rgba(0, 0, 0, 0);color:red; border: none");
        }

        // Score
        sString = QString("%1").arg(iScore[iTeam], 2);
        pScoreEdit[iTeam] = new Edit(sString, iTeam);
        pScoreEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        pScoreEdit[iTeam]->setMaxLength(2);
        pScoreEdit[iTeam]->setPalette(pal);
        pScoreEdit[iTeam]->setReadOnly(true);

        // Score buttons
        pScoreIncrement[iTeam] = new Button("", iTeam);
        pScoreIncrement[iTeam]->setIcon(plusButtonIcon);
        pScoreIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        pScoreDecrement[iTeam] = new Button("", iTeam);
        pScoreDecrement[iTeam]->setIcon(minusButtonIcon);
        pScoreDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iScore[iTeam] == 0)
            pScoreDecrement[iTeam]->setEnabled(false);
    }

    // Period
    pPeriodLabel = new QLabel(tr("Periodo"));
    pPeriodLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    sString = QString("%1").arg(iPeriod, 1);
    pPeriodEdit = new Edit(sString, 0);
    pPeriodEdit->setAlignment(Qt::AlignHCenter);
    pPeriodEdit->setMaxLength(1);
    pPeriodEdit->setPalette(pal);
    pPeriodEdit->setReadOnly(true);

    // Timeout
    pTimeoutLabel = new QLabel(tr("Timeout"));
    pTimeoutLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    // Start/Stop Count buttons
    QPixmap pixmap;
    QIcon ButtonIcon;
    pixmap.load(":/ButtonIcons/Go.png");
    ButtonIcon.addPixmap(pixmap);
    pCountStart = new Button("", 0);
    pCountStart->setIcon(ButtonIcon);
    pCountStart->setIconSize(pixmap.rect().size());

    pixmap.load(":/ButtonIcons/StopTime.png");
    ButtonIcon.addPixmap(pixmap);
    pCountStop = new Button("", 0);
    pCountStop->setIcon(ButtonIcon);
    pCountStop->setIconSize(pixmap.rect().size());

    // Time Count
    QString sRemainingTime;
    lldiv_t iRes = div(remainingMilliSeconds, qint64(60000));
    sRemainingTime = QString("%1:%2")
                         .arg(iRes.quot, 1)
                         .arg(int(iRes.rem), 2, 10, QChar('0'));
    pTimeEdit = new Edit(sRemainingTime, 0);
    pTimeEdit->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    pTimeEdit->setMaxLength(4);
    pTimeEdit->setPalette(pal);

    // Score
    pScoreLabel = new QLabel(tr("Punti"));
    pScoreLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}


void
WaterpoloController::disableUi() {
    for(int i=0; i<2; i++) {
        pTeamName[i]->setDisabled(true);
        pTimeoutEdit[i]->setDisabled(true);
        pScoreEdit[i]->setDisabled(true);
        pTimeoutIncrement[i]->setDisabled(true);
        pTimeoutDecrement[i]->setDisabled(true);
        pScoreIncrement[i]->setDisabled(true);
        pScoreDecrement[i]->setDisabled(true);
    }
    pPeriodEdit->setDisabled(true);
    pTimeEdit->setDisabled(true);

    pNewPeriodButton->setDisabled(true);
    pChangeFieldButton->setDisabled(true);

    disableGeneralButtons();
}


void
WaterpoloController::enableUi() {
    for(int i=0; i<2; i++) {
        pTeamName[i]->setEnabled(true);
        pTimeoutEdit[i]->setEnabled(true);
        pScoreEdit[i]->setEnabled(true);
        if(iTimeout[i] < gsArgs.maxTimeout) {
            pTimeoutIncrement[i]->setEnabled(true);
        }
        if(iTimeout[i] > 0) {
            pTimeoutDecrement[i]->setEnabled(true);
        }
        pScoreIncrement[i]->setEnabled(true);
        pScoreDecrement[i]->setEnabled(true);
    }
    pPeriodEdit->setEnabled(true);
    pTimeEdit->setEnabled(true);

    pNewPeriodButton->setEnabled(true);
    pChangeFieldButton->setEnabled(true);

    enableGeneralButtons();
}



void
WaterpoloController::setEventHandlers() {
    for(int iTeam=0; iTeam <2; iTeam++) {
        connect(pTimeoutIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutIncrement(int)));
        connect(pTimeoutDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutDecrement(int)));
        connect(pScoreIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreIncrement(int)));
        connect(pScoreDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreDecrement(int)));
    }
    // Start/Stop Count
    connect(pCountStart, SIGNAL(buttonClicked(int)),
            this, SLOT(onCountStart(int)));
    connect(pCountStop, SIGNAL(buttonClicked(int)),
            this, SLOT(onCountStop(int)));
    // Time editing
    connect(pTimeEdit, SIGNAL(mousePressed()),
            this, SLOT(onGameTimeChanging()));
    // New Period
    connect(pNewPeriodButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewPeriodClicked()));
    // Exchange Field Position
    connect(pChangeFieldButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonChangeFieldClicked()));
}


// =========================
// Event management routines
// =========================

void
WaterpoloController::onGameTimeChanging() {
    QString sTime = pTimeEdit->text();
    int minutes = sTime.left(1).toInt();
    int seconds = sTime.right(2).toInt();
    pRemainingTimeDialog->setMinutes(minutes);
    pRemainingTimeDialog->setSeconds(seconds);
    if(pRemainingTimeDialog->exec() == QDialog::Accepted) {
        minutes = pRemainingTimeDialog->getMinutes();
        seconds = pRemainingTimeDialog->getSeconds();
        QString sMessage = QString("<newTime>%1:%2</newTime>")
                               .arg(minutes).arg(seconds);
        sendMessage(sMessage);
    }
    pCountStart->setFocus(); // Per evitare che il focus vada altrove
}


void
WaterpoloController::onButtonNewPeriodClicked() {
    if(iPeriod == gsArgs.maxPeriods) {
        QMessageBox::information(this, tr("WaterPolo_Controller"),
                                 tr("Massimo Numero di periodi Raggiunto"));
        return;
    }
    int iRes = QMessageBox::question(this, tr("WaterPolo_Controller"),
                                     tr("Vuoi davvero iniziare un nuovo Periodo ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;
    QString sMessage = QString("<period>%1</period>")
                           .arg(iPeriod+1);
    sendMessage(sMessage);
    pPeriodEdit->setFocus(); // Per evitare che il focus vada altrove
}


void
WaterpoloController::onTimeOutIncrement(int iTeam) {
    QString sText = QString("<inctimeout>%1</inctimeout>")
                        .arg(iTeam,1);
    sendMessage(sText);
}


void
WaterpoloController::onTimeOutDecrement(int iTeam) {
    QString sText = QString("<dectimeout>%1</dectimeout>")
                        .arg(iTeam,1);
    sendMessage(sText);
}


void
WaterpoloController::onCountStart(int iTeam) {
    Q_UNUSED(iTeam)
    QString sText = QString("<startT>%1</startT>")
                        .arg(1,1);
    sendMessage(sText);
}


void
WaterpoloController::onCountStop(int iTeam) {
    Q_UNUSED(iTeam)
    QString sText = QString("<stopT>%1</stopT>")
                        .arg(1,1);
    sendMessage(sText);
}


void
WaterpoloController::onScoreIncrement(int iTeam) {
    QString sText = QString("<incscore>%1</incscore>")
                    .arg(iTeam,1);
    sendMessage(sText);
}


void
WaterpoloController::onScoreDecrement(int iTeam) {
    QString sText = QString("<decscore>%1</decscore>")
                        .arg(iTeam,1);
    sendMessage(sText);
}


void
WaterpoloController::onButtonChangeFieldClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Scambiare il campo delle squadre ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    QString sText = QString("<fieldExchange>1</fieldExchange>");
    sendMessage(sText);
}


void
WaterpoloController::onChangePanelOrientation(PanelOrientation orientation) {
    Q_UNUSED(orientation)
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Direction %1")
                   .arg(static_cast<int>(orientation)));
#endif
    QString sMessage = QString("<setOrientation>%1</setOrientation>")
                           .arg(static_cast<int>(orientation));
    sendMessage(sMessage);
}


void
WaterpoloController::processTextMessage(QString sMessage) {
    QString sToken;
    bool ok;
    int iVal;
    QString sNoData = QString("NoData");

    sToken = XML_Parse(sMessage, "time");
    if(sToken != sNoData){
        pTimeEdit->setText(sToken);
        if(sToken==QString("0:00")) {
            enableUi();
            pCountStart->setDisabled(true);
            pCountStop->setDisabled(true);
        }
    }// time

    sToken = XML_Parse(sMessage, "startT");
    if(sToken != sNoData){
        pCountStart->setDisabled(true);
        pCountStop->setEnabled(true);
        disableUi();
    }// start time

    sToken = XML_Parse(sMessage, "stopT");
    if(sToken != sNoData){
        pCountStart->setEnabled(true);
        pCountStop->setDisabled(true);
        enableUi();
    }// start time

    sToken = XML_Parse(sMessage, "period");
    if(sToken != sNoData){
        pPeriodEdit->setText(sToken);
        iPeriod = sToken.toInt();
    }// period

    sToken = XML_Parse(sMessage, "team0");
    if(sToken != sNoData){
        pTeamName[0]->setText(sToken.left(maxTeamNameLen));
    }// team0

    sToken = XML_Parse(sMessage, "team1");
    if(sToken != sNoData){
        pTeamName[1]->setText(sToken.left(maxTeamNameLen));
    }// team1

    sToken = XML_Parse(sMessage, "timeout0");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>2)
            iVal = 8;
        pTimeoutEdit[0]->setText(QString("%1"). arg(iVal));
        pTimeoutDecrement[0]->setEnabled((iVal > 0));
        pTimeoutIncrement[0]->setEnabled((iVal != gsArgs.maxTimeout));
    }// timeout0

    sToken = XML_Parse(sMessage, "timeout1");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>2)
            iVal = 8;
        pTimeoutEdit[1]->setText(QString("%1"). arg(iVal));
        pTimeoutDecrement[1]->setEnabled((iVal > 0));
        pTimeoutIncrement[1]->setEnabled((iVal != gsArgs.maxTimeout));
    }// timeout1

    sToken = XML_Parse(sMessage, "score0");
    if(sToken != sNoData) {
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>99)
            iVal = 99;
        pScoreEdit[0]->setText(QString("%1").arg(iVal));
        pScoreDecrement[0]->setEnabled((iVal > 0));
        pScoreIncrement[0]->setEnabled((iVal != 99));
    }// score0

    sToken = XML_Parse(sMessage, "score1");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>99)
            iVal = 99;
        pScoreEdit[1]->setText(QString("%1").arg(iVal));
        pScoreDecrement[1]->setEnabled((iVal > 0));
        pScoreIncrement[1]->setEnabled((iVal != 99));
    }// score1

    sToken = XML_Parse(sMessage, "newGame");
    if(sToken != sNoData){
        pCountStart->setEnabled(true);
    }// score1

    BtScoreController::processGeneralMessages(sMessage);
}
