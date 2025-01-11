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

#include <QSettings>
#include <QGridLayout>
#include <QMessageBox>
#include <QResizeEvent>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QScreen>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

#include "../CommonFiles/btserver.h"
#include "../CommonFiles/edit.h"
#include "../CommonFiles/button.h"
#include "../CommonFiles/utility.h"
#include "waterpoloctrl.h"
#include "generalsetupdialog.h"
#include "waterpolopanel.h"


WaterPoloCtrl::WaterPoloCtrl(QFile *myLogFile, QWidget *parent)
    : ScoreController(myLogFile, parent)
    , pWaterPoloPanel(new WaterPoloPanel(myLogFile))
    , pRemainingTimeDialog(new RemainingTimeDialog)
    , iPeriod(1)
    , bFontBuilt(false)
    , maxTeamNameLen(15)
    , runMilliSeconds(0)
    , lastM(-1)
    , lastS(-1)
{
    setWindowTitle("Waterpolo Controller - © Gabriele Salvato (2025)");
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

    pWaterPoloPanel->showFullScreen();
    updateTimer.setTimerType(Qt::PreciseTimer);

    setEventHandlers();
    pCountStop->setDisabled(true);
    startTimer.setSingleShot(true);
    startTimer.start(100);
}


void
WaterPoloCtrl::closeEvent(QCloseEvent *event) {
    updateTimer.disconnect();
    updateTimer.stop();
    SaveSettings();
    ScoreController::closeEvent(event);
    if(pWaterPoloPanel) {
        pWaterPoloPanel->deleteLater();
    }
    event->accept();
}


void
WaterPoloCtrl::resizeEvent(QResizeEvent *event) {
    QList<QScreen*> screens = QApplication::screens();
    QRect screenRect = screens.at(0)->geometry();
    QRect myRect = frameGeometry();
    int x0 = (screenRect.width() - myRect.width())/2;
    int y0 = (screenRect.height()-myRect.height())/2;
    move(x0,y0);
    if(!bFontBuilt) {
        bFontBuilt = true;
        buildFontSizes();
        event->setAccepted(true);
    }
}


void
WaterPoloCtrl::GeneralSetup() {
    GeneralSetupDialog* pGeneralSetupDialog = new GeneralSetupDialog(&gsArgs);
    connect(pGeneralSetupDialog, SIGNAL(changeOrientation(PanelOrientation)),
            this, SLOT(onChangePanelOrientation(PanelOrientation)));
    int iResult = pGeneralSetupDialog->exec();
    if(iResult == QDialog::Accepted) {
        if(!gsArgs.sSlideDir.endsWith(QString("/")))
            gsArgs.sSlideDir+= QString("/");
        QDir slideDir(gsArgs.sSlideDir);
        if(!slideDir.exists()) {
            gsArgs.sSlideDir = QStandardPaths::displayName(QStandardPaths::PicturesLocation);
        }
        if(!gsArgs.sSpotDir.endsWith(QString("/")))
            gsArgs.sSpotDir+= QString("/");
        QDir spotDir(gsArgs.sSpotDir);
        if(!spotDir.exists()) {
            gsArgs.sSpotDir = QStandardPaths::displayName(QStandardPaths::MoviesLocation);
        }
        remainingMilliSeconds = gsArgs.iTimeDuration * 60000;
        QString sRemainingTime;
        lldiv_t iRes = div(remainingMilliSeconds+999, 60000LL);
        int iMinutes = int(iRes.quot);
        int iSeconds = int(iRes.rem/1000);
        sRemainingTime = QString("%1:%2")
                             .arg(iMinutes, 1)
                             .arg(iSeconds, 2, 10, QChar('0'));
        pTimeEdit->setText(sRemainingTime);
        SaveSettings();
        sendAll();
    }
    delete pGeneralSetupDialog;
    pGeneralSetupDialog = nullptr;
}


void
WaterPoloCtrl::buildFontSizes() {
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
WaterPoloCtrl::setWindowLayout() {
    QWidget *widget = new QWidget();
    auto *mainLayout = new QGridLayout();

    int gamePanelWidth  = 15;
    int gamePanelHeigth =  8;

    mainLayout->addLayout(CreateGamePanel(),
                          0,
                          0,
                          gamePanelHeigth,
                          gamePanelWidth);

    mainLayout->addLayout(CreateGameButtons(),
                          gamePanelHeigth,
                          0,
                          2,
                          8);

    mainLayout->addLayout(pSpotButtonsLayout,
                          gamePanelHeigth,
                          8,
                          2,
                          gamePanelWidth-5);

    widget->setLayout(mainLayout);
    setCentralWidget(widget);
    setTabOrder(pTeamName[0], pTeamName[1]);
}


QGridLayout*
WaterPoloCtrl::CreateGamePanel() {
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
WaterPoloCtrl::CreateGameButtons() {
    auto* gameButtonLayout = new QHBoxLayout();
    QSize iconSize = QSize(48,48);

    QPixmap* pPixmap = new QPixmap(":/ButtonIcons/Exchange.png");
    pChangeFieldButton = new QPushButton(QIcon(*pPixmap), "");
    pChangeFieldButton->setIconSize(iconSize);
    pChangeFieldButton->setFlat(true);
    pChangeFieldButton->setToolTip("Inverti Campo");

    pPixmap->load(":/ButtonIcons/water-polo-ball.png");
    pNewGameButton = new QPushButton(QIcon(*pPixmap), "");
    pNewGameButton->setIconSize(iconSize);
    pNewGameButton->setFlat(true);
    pNewGameButton->setToolTip("Nuova Partita");

    pPixmap->load(":/ButtonIcons/NewPeriod.png");
    pNewPeriodButton  = new QPushButton(*pPixmap, "");
    pNewPeriodButton->setIconSize(iconSize);
    pNewPeriodButton->setFlat(true);
    pNewPeriodButton->setToolTip("Nuovo Periodo");

    delete pPixmap;

    gameButtonLayout->addWidget(pNewGameButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pNewPeriodButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pChangeFieldButton);
    gameButtonLayout->addStretch();
    return gameButtonLayout;
}


void
WaterPoloCtrl::GetSettings() {
    gsArgs.maxTimeout           = pSettings->value("waterpolo/maxTimeout", 2).toInt();
    gsArgs.maxPeriods           = pSettings->value("waterpolo/maxPeriods", 4).toInt();
    gsArgs.iTimeDuration        = pSettings->value("waterpolo/TimeDuration", 8).toInt();
    gsArgs.sSlideDir            = pSettings->value("directories/slides", gsArgs.sSlideDir).toString();
    gsArgs.sSpotDir             = pSettings->value("directories/spots",  gsArgs.sSpotDir).toString();
    gsArgs.isPanelMirrored      = pSettings->value("panel/orientation",  true).toBool();
    gsArgs.sTeamLogoFilePath[0] = pSettings->value("panel/logo0", ":/../CommonFiles/Logo_UniMe.png").toString();
    gsArgs.sTeamLogoFilePath[1] = pSettings->value("panel/logo1", ":/../CommonFiles/Logo_SSD_UniMe.png").toString();
    gsArgs.sTeam[0]             = pSettings->value("team1/name", QString(tr("Locali"))).toString();
    gsArgs.sTeam[1]             = pSettings->value("team2/name", QString(tr("Ospiti"))).toString();

    iTimeout[0] = pSettings->value("team1/timeouts", 0).toInt();
    iTimeout[1] = pSettings->value("team2/timeouts", 0).toInt();
    iScore[0]   = pSettings->value("team1/score", 0).toInt();
    iScore[1]   = pSettings->value("team2/score", 0).toInt();
    iPeriod     = pSettings->value("game/period", 1).toInt();

    remainingMilliSeconds = gsArgs.iTimeDuration * 60000;
    runMilliSeconds = 0;

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
WaterPoloCtrl::disableUi() {
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
    pPeriodIncrement->setDisabled(true);
    pPeriodDecrement->setDisabled(true);

    pNewPeriodButton->setDisabled(true);
    pNewGameButton->setDisabled(true);
    pChangeFieldButton->setDisabled(true);

    disableGeneralButtons();
}


void
WaterPoloCtrl::enableUi() {
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
    pPeriodIncrement->setEnabled(true);
    pPeriodDecrement->setEnabled(true);

    pNewPeriodButton->setEnabled(true);
    pNewGameButton->setEnabled(true);
    pChangeFieldButton->setEnabled(true);

    enableGeneralButtons();
}


void
WaterPoloCtrl::sendAll() {
    for(int i=0; i<2; i++) {
        pWaterPoloPanel->setTeam(i, pTeamName[i]->text());
        pWaterPoloPanel->setScore(i, iScore[i]);
    }
    pWaterPoloPanel->setLogo(0, gsArgs.sTeamLogoFilePath[0]);
    pWaterPoloPanel->setLogo(1, gsArgs.sTeamLogoFilePath[1]);
    pWaterPoloPanel->setMirrored(gsArgs.isPanelMirrored);
    pWaterPoloPanel->setPeriod(iPeriod);
    pWaterPoloPanel->setTime(pTimeEdit->text());
    btSendAll();
}


void
WaterPoloCtrl::btSendAll() {
    QString sMessage = QString();

    sMessage = QString("<setOrientation>%1</setOrientation>")
                   .arg(static_cast<int>(gsArgs.isPanelMirrored));
    pBtServer->sendMessage(sMessage);

    for(int i=0; i<2; i++) {
        sMessage = QString("<team%1>%2</team%3>")
        .arg(i,1)
            .arg(pTeamName[i]->text().toLocal8Bit().data())
            .arg(i,1);
        pBtServer->sendMessage(sMessage);
        sMessage = QString("<timeout%1>%2</timeout%3>")
                       .arg(i,1)
                       .arg(iTimeout[i])
                       .arg(i,1);
        pBtServer->sendMessage(sMessage);
        sMessage = QString("<score%1>%2</score%3>")
                       .arg(i,1)
                       .arg(iScore[i], 2)
                       .arg(i,1);
        pBtServer->sendMessage(sMessage);
    }
    // Time
    sMessage = QString("<time>%1</time>").arg(pTimeEdit->text());
    pBtServer->sendMessage(sMessage);
    sMessage = QString("<startTime>%1</startTime>").arg(pTimeEdit->text());
    pBtServer->sendMessage(sMessage);
    // Period
    sMessage = QString("<period>%1</period>")
                           .arg(pPeriodEdit->text(), 1);
    pBtServer->sendMessage(sMessage);
    // Slideshow/Video
    if(myStatus == showSlides)
        sMessage = QString("<slideshow>1</slideshow>");
    else if(myStatus == showSpots)
        sMessage = QString("<spotloop>1</spotloop>");
    pBtServer->sendMessage(sMessage);
}


void
WaterPoloCtrl::SaveStatus() {
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
WaterPoloCtrl::SaveSettings() { // Save General Setup Values
    pSettings->setValue("directories/slides",        gsArgs.sSlideDir);
    pSettings->setValue("directories/spots",         gsArgs.sSpotDir);
    pSettings->setValue("waterpolo/maxTimeout",      gsArgs.maxTimeout);
    pSettings->setValue("waterpolo/maxPeriods",      gsArgs.maxPeriods);
    pSettings->setValue("waterpolo/TimeDuration",    gsArgs.iTimeDuration);
    pSettings->setValue("panel/orientation",         gsArgs.isPanelMirrored);
    pSettings->setValue("panel/logo0",               gsArgs.sTeamLogoFilePath[0]);
    pSettings->setValue("panel/logo1",               gsArgs.sTeamLogoFilePath[1]);
}


void
WaterPoloCtrl::buildControls() {
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

    //Period buttons
    pPeriodIncrement = new Button("", 0);
    pPeriodIncrement->setIcon(plusButtonIcon);
    pPeriodIncrement->setIconSize(plusPixmap.rect().size());
    pPeriodDecrement = new Button("", 0);
    pPeriodDecrement->setIcon(minusButtonIcon);
    pPeriodDecrement->setIconSize(minusPixmap.rect().size());
    if(iPeriod == 1)
        pPeriodDecrement->setDisabled(true);
    if(iPeriod == gsArgs.maxPeriods)
        pPeriodIncrement->setDisabled(true);

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
WaterPoloCtrl::setEventHandlers() {
    connect(&startTimer, SIGNAL(timeout()),
            this, SLOT(onAppStart()));
    connect(&updateTimer, SIGNAL(timeout()),
            this, SLOT(onTimeUpdate()));
    for(int iTeam=0; iTeam <2; iTeam++) {
        connect(pTeamName[iTeam], SIGNAL(teamTextChanged(QString,int)),
                this, SLOT(onTeamTextChanged(QString,int)));
        connect(pTimeoutIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutIncrement(int)));
        connect(pTimeoutDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutDecrement(int)));
        connect(pScoreIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreIncrement(int)));
        connect(pScoreDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreDecrement(int)));
    }
    // Period Buttons
    connect(pPeriodIncrement, SIGNAL(buttonClicked(int)),
            this, SLOT(onPeriodIncrement(int)));
    connect(pPeriodDecrement, SIGNAL(buttonClicked(int)),
            this, SLOT(onPeriodDecrement(int)));
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
    // New Game
    connect(pNewGameButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewGameClicked()));
    // Exchange Field Position
    connect(pChangeFieldButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonChangeFieldClicked()));
}


// =========================
// Event management routines
// =========================

void
WaterPoloCtrl::onAppStart() {
    sendAll();
    updateTimer.start(20);
}


void
WaterPoloCtrl::onGameTimeChanging() {
    QString sTime = pTimeEdit->text();
    int minutes = sTime.left(1).toInt();
    int seconds = sTime.right(2).toInt();
    pRemainingTimeDialog->setMinutes(minutes);
    pRemainingTimeDialog->setSeconds(seconds);
    lastM = minutes;
    lastS = seconds;
    if(pRemainingTimeDialog->exec() == QDialog::Accepted) {
        minutes = pRemainingTimeDialog->getMinutes();
        seconds = pRemainingTimeDialog->getSeconds();
        qint64 mSecToGo = (minutes*60+seconds)*1000;
        if(mSecToGo <= gsArgs.iTimeDuration*60000) {
            runMilliSeconds = gsArgs.iTimeDuration * 60000-mSecToGo;
            if(runMilliSeconds >= 0) {
                QString sRemainingTime = QString("%1:%2")
                                             .arg(minutes, 1)
                                             .arg(seconds, 2, 10, QChar('0'));
                pTimeEdit->setText(sRemainingTime);
                pCountStart->setEnabled(true);
                QString sMessage = QString("<time>%1</time>")
                                       .arg(sRemainingTime);
                pBtServer->sendMessage(sMessage);
            }
        }
    }
    pCountStart->setFocus();
    // Settare il Focus su un altro controllo
}


void
WaterPoloCtrl::onTimeUpdate() {
    if(tempoTimer.isValid()) {
        qint64 currentMilli = tempoTimer.elapsed();
        currentMilli += runMilliSeconds;
        qint64 timeToStop = remainingMilliSeconds-currentMilli;
        if(timeToStop<=0) {
            pCountStart->setDisabled(true);
            pCountStop->setDisabled(true);
            timeToStop = 0;
            tempoTimer.invalidate();
            enableUi();
        }
        QString sRemainingTime;
        lldiv_t iRes = div(timeToStop+999, 60000LL);
        int iMinutes = int(iRes.quot);
        int iSeconds = int(iRes.rem/1000);
        if((lastS != iSeconds) || (lastM != iMinutes)) {
            sRemainingTime = QString("%1:%2")
                                 .arg(iMinutes, 1)
                                 .arg(iSeconds, 2, 10, QChar('0'));
            pTimeEdit->setText(sRemainingTime);
            pWaterPoloPanel->setTime(sRemainingTime);
            QString sMessage = QString("<time>%1</time>")
                                   .arg(sRemainingTime);
            pBtServer->sendMessage(sMessage);
            lastS = iSeconds;
            lastM = iMinutes;
        }
    }
}


void
WaterPoloCtrl::onPeriodIncrement(int) {
    iPeriod++;
    if(iPeriod >= gsArgs.maxPeriods) {
        pPeriodIncrement->setEnabled(false);
    }
    pPeriodDecrement->setEnabled(true);
    pWaterPoloPanel->setPeriod(iPeriod);
    QString sMessage = QString("<period>%1</period>")
                           .arg(iPeriod);
    pBtServer->sendMessage(sMessage);
    QString sText = QString("%1").arg(iPeriod);
    pPeriodEdit->setText(sText);
    sText = QString("game/period");
    pSettings->setValue(sText, iPeriod);
    pPeriodEdit->setFocus(); // Per evitare che il focus vada altrove
}


void
WaterPoloCtrl::onPeriodDecrement(int) {
    iPeriod--;
    if(iPeriod == 1) {
        pPeriodDecrement->setEnabled(false);
    }
    pPeriodIncrement->setEnabled(true);
    pWaterPoloPanel->setPeriod(iPeriod);
    QString sMessage = QString("<period>%1</period>")
                           .arg(iPeriod);
    pBtServer->sendMessage(sMessage);
    QString sText = QString("%1").arg(iPeriod);
    pPeriodEdit->setText(sText);
    sText = QString("game/period");
    pSettings->setValue(sText, iPeriod);
    pPeriodEdit->setFocus(); // Per evitare che il focus vada altrove
}


void
WaterPoloCtrl::onTimeOutIncrement(int iTeam) {
    iTimeout[iTeam]++;
    if(iTimeout[iTeam] >= gsArgs.maxTimeout) {
        pTimeoutIncrement[iTeam]->setEnabled(false);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:red; border: none");
    }
    pTimeoutDecrement[iTeam]->setEnabled(true);
    // pWaterPoloPanel->setTimeout(iTeam, iTimeout[iTeam]);
    QString sMessage = QString("<timeout%1>%2</timeout%3>")
                           .arg(iTeam,1)
                           .arg(iTimeout[iTeam])
                           .arg(iTeam,1);
    pBtServer->sendMessage(sMessage);
    QString sText = QString("%1").arg(iTimeout[iTeam]);
    pTimeoutEdit[iTeam]->setText(sText);
    sText = QString("team%1/timeouts").arg(iTeam+1, 1);
    pSettings->setValue(sText, iTimeout[iTeam]);
    pTimeoutEdit[iTeam]->setFocus(); // Per evitare che il focus vada all'edit delle squadre
}


void
WaterPoloCtrl::onTimeOutDecrement(int iTeam) {
    iTimeout[iTeam]--;
    if(iTimeout[iTeam] == 0) {
        pTimeoutDecrement[iTeam]->setEnabled(false);
    }
    pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
    pTimeoutIncrement[iTeam]->setEnabled(true);
    // pWaterPoloPanel->setTimeout(iTeam, iTimeout[iTeam]);
    QString sMessage = QString("<timeout%1>%2</timeout%3>")
                           .arg(iTeam,1)
                           .arg(iTimeout[iTeam])
                           .arg(iTeam,1);
    pBtServer->sendMessage(sMessage);
    QString sText;
    sText = QString("%1").arg(iTimeout[iTeam], 1);
    pTimeoutEdit[iTeam]->setText(sText);
    sText = QString("team%1/timeouts").arg(iTeam+1, 1);
    pSettings->setValue(sText, iTimeout[iTeam]);
}


void
WaterPoloCtrl::onCountStart(int iTeam) {
    Q_UNUSED(iTeam)
    tempoTimer.restart();
    pCountStart->setDisabled(true);
    pCountStop->setEnabled(true);
    disableUi();
    QString sMessage = QString("<startT>%1</startT>").arg(0, 1);
    pBtServer->sendMessage(sMessage);
}


void
WaterPoloCtrl::onCountStop(int iTeam) {
    Q_UNUSED(iTeam)
    runMilliSeconds += tempoTimer.elapsed();
    tempoTimer.invalidate();
    pCountStart->setEnabled(true);
    pCountStop->setDisabled(true);
    pTimeEdit->setEnabled(true);
    enableUi();
    QString sMessage = QString("<stopT>%1</stopT>").arg(0, 1);
    pBtServer->sendMessage(sMessage);
}


void
WaterPoloCtrl::onScoreIncrement(int iTeam) {
    iScore[iTeam]++;
    pScoreDecrement[iTeam]->setEnabled(true);
    if(iScore[iTeam] > 98) {
        pScoreIncrement[iTeam]->setEnabled(false);
    }
    pWaterPoloPanel->setScore(iTeam, iScore[iTeam]);
    QString sMessage = QString("<score%1>%2</score%3>")
                           .arg(iTeam,1)
                           .arg(iScore[iTeam], 2)
                           .arg(iTeam,1);
    pBtServer->sendMessage(sMessage);
    QString sText;
    sText = QString("%1").arg(iScore[iTeam], 2);
    pScoreEdit[iTeam]->setText(sText);
    sText = QString("team%1/score").arg(iTeam+1, 1);
    pSettings->setValue(sText, iScore[iTeam]);
}


void
WaterPoloCtrl::onScoreDecrement(int iTeam) {
    iScore[iTeam]--;
    pScoreIncrement[iTeam]->setEnabled(true);
    if(iScore[iTeam] == 0) {
        pScoreDecrement[iTeam]->setEnabled(false);
    }
    pWaterPoloPanel->setScore(iTeam, iScore[iTeam]);
    QString sMessage = QString("<score%1>%2</score%3>")
                           .arg(iTeam,1)
                           .arg(iScore[iTeam], 2)
                           .arg(iTeam,1);
    pBtServer->sendMessage(sMessage);
    QString sText;
    sText = QString("%1").arg(iScore[iTeam], 2);
    pScoreEdit[iTeam]->setText(sText);
    sText = QString("team%1/score").arg(iTeam+1, 1);
    pSettings->setValue(sText, iScore[iTeam]);
}


void
WaterPoloCtrl::onTeamTextChanged(QString sText, int iTeam) {
    gsArgs.sTeam[iTeam] = sText;
    pWaterPoloPanel->setTeam(iTeam, gsArgs.sTeam[iTeam]);
    QString sMessage = QString("<team%1>%2</team%3>")
                           .arg(iTeam,1)
                           .arg(gsArgs.sTeam[iTeam])
                           .arg(iTeam,1);
    pBtServer->sendMessage(sMessage);
    sText = QString("team%1/name").arg(iTeam+1, 1);
    pSettings->setValue(sText, gsArgs.sTeam[iTeam]);
}


void
WaterPoloCtrl::onButtonChangeFieldClicked() {
    int iRes = QMessageBox::question(this, tr("Waterpolo_Controller"),
                                     tr("Scambiare il campo delle squadre ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;
    exchangeField();
}


void
WaterPoloCtrl::exchangeField() {
    QString sText = gsArgs.sTeam[0];
    gsArgs.sTeam[0] = gsArgs.sTeam[1];
    gsArgs.sTeam[1] = sText;
    pTeamName[0]->setText(gsArgs.sTeam[0]);
    pTeamName[1]->setText(gsArgs.sTeam[1]);

    sText = gsArgs.sTeamLogoFilePath[0];
    gsArgs.sTeamLogoFilePath[0] = gsArgs.sTeamLogoFilePath[1];
    gsArgs.sTeamLogoFilePath[1] = sText;

    int iVal;
    iVal = iScore[0];
    iScore[0] = iScore[1];
    iScore[1] = iVal;
    sText = QString("%1").arg(iScore[0], 2);
    pScoreEdit[0]->setText(sText);
    sText = QString("%1").arg(iScore[1], 2);
    pScoreEdit[1]->setText(sText);

    iVal = iTimeout[0];
    iTimeout[0] = iTimeout[1];
    iTimeout[1] = iVal;
    sText = QString("%1").arg(iTimeout[0]);
    pTimeoutEdit[0]->setText(sText);
    sText = QString("%1").arg(iTimeout[1]);
    pTimeoutEdit[1]->setText(sText);

    for(int iTeam=0; iTeam<2; iTeam++) {
        pScoreDecrement[iTeam]->setEnabled(true);
        pScoreIncrement[iTeam]->setEnabled(true);
        if(iScore[iTeam] == 0) {
            pScoreDecrement[iTeam]->setEnabled(false);
        }
        if(iScore[iTeam] > 98) {
            pScoreIncrement[iTeam]->setEnabled(false);
        }

        pTimeoutIncrement[iTeam]->setEnabled(true);
        pTimeoutDecrement[iTeam]->setEnabled(true);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
        if(iTimeout[iTeam] == gsArgs.maxTimeout) {
            pTimeoutIncrement[iTeam]->setEnabled(false);
            pTimeoutEdit[iTeam]->setStyleSheet("background:rgba(0, 0, 0, 0);color:white; border: none");
        }
        if(iTimeout[iTeam] == 0) {
            pTimeoutDecrement[iTeam]->setEnabled(false);
        }
    }
    sendAll();
    SaveStatus();
}


void
WaterPoloCtrl::onButtonNewPeriodClicked() {
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
    startNewPeriod();
}


void
WaterPoloCtrl::startNewPeriod() {
    iPeriod++;
    if(iPeriod >= gsArgs.maxPeriods) {
        pPeriodIncrement->setEnabled(false);
    }
    pPeriodDecrement->setEnabled(true);
    pCountStart->setEnabled(true);
    pCountStop->setDisabled(true);
    QString sText = QString("%1").arg(iPeriod);
    pPeriodEdit->setText(sText);
    if(iPeriod == 3) { // Le squadre compresi i giocatori, allenatori e dirigenti,
                       // cambiano campo prima dell’inizio del 3° tempo.
        // Exchange team's order in the field
        QString sText = gsArgs.sTeam[0];
        gsArgs.sTeam[0] = gsArgs.sTeam[1];
        gsArgs.sTeam[1] = sText;
        pTeamName[0]->setText(gsArgs.sTeam[0]);
        pTeamName[1]->setText(gsArgs.sTeam[1]);

        sText = gsArgs.sTeamLogoFilePath[0];
        gsArgs.sTeamLogoFilePath[0] = gsArgs.sTeamLogoFilePath[1];
        gsArgs.sTeamLogoFilePath[1] = sText;
    }

    for(int iTeam=0; iTeam<2; iTeam++) {
        iTimeout[iTeam] = 0;
        sText = QString("%1").arg(iTimeout[iTeam], 1);
        pTimeoutEdit[iTeam]->setText(sText);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
        pTimeoutDecrement[iTeam]->setEnabled(false);
        pTimeoutIncrement[iTeam]->setEnabled(true);
    }
    remainingMilliSeconds = gsArgs.iTimeDuration * 60000;
    runMilliSeconds = 0;

    QString sRemainingTime;
    lldiv_t iRes = div(remainingMilliSeconds+999, 60000LL);
    int iMinutes = int(iRes.quot);
    int iSeconds = int(iRes.rem/1000);
    sRemainingTime = QString("%1:%2").arg(iMinutes, 1)
                         .arg(iSeconds, 2, 10, QChar('0'));
    pTimeEdit->setText(sRemainingTime);
    sendAll();
    btSendAll();
    SaveStatus();
}


void
WaterPoloCtrl::onButtonNewGameClicked() {
    int iResponse = QMessageBox::question(this, tr("WaterPolo_Controller"),
                                     tr("Iniziare una Nuova Partita ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iResponse != QMessageBox::Yes) return;

    gsArgs.sTeam[0]    = tr("Locali");
    gsArgs.sTeam[1]    = tr("Ospiti");
    QString sText;
    for(int iTeam=0; iTeam<2; iTeam++) {
        pTeamName[iTeam]->setText(gsArgs.sTeam[iTeam]);
        iTimeout[iTeam] = 0;
        sText = QString("%1").arg(iTimeout[iTeam], 1);
        pTimeoutEdit[iTeam]->setText(sText);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
        iScore[iTeam]   = 0;
        sText = QString("%1").arg(iScore[iTeam], 2);
        pScoreEdit[iTeam]->setText(sText);
        pTimeoutDecrement[iTeam]->setEnabled(false);
        pTimeoutIncrement[iTeam]->setEnabled(true);
        pScoreDecrement[iTeam]->setEnabled(false);
        pScoreIncrement[iTeam]->setEnabled(true);
        pCountStart->setEnabled(true);
    }
    iPeriod = 1;
    pPeriodEdit->setText(QString("%1").arg(iPeriod));
    remainingMilliSeconds = gsArgs.iTimeDuration * 60000;
    runMilliSeconds = 0;
    QString sRemainingTime;
    lldiv_t iRes = div(remainingMilliSeconds+999, 60000LL);
    int iMinutes = int(iRes.quot);
    int iSeconds = int(iRes.rem/1000);
    sRemainingTime = QString("%1:%2").arg(iMinutes, 1)
                         .arg(iSeconds, 2, 10, QChar('0'));
    pTimeEdit->setText(sRemainingTime);
    sendAll();
    SaveStatus();
    sText = QString("<newGame>0</newGame>");
    pBtServer->sendMessage(sText);
}


void
WaterPoloCtrl::onChangePanelOrientation(PanelOrientation orientation) {
    Q_UNUSED(orientation)
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Direction %1")
                   .arg(static_cast<int>(orientation)));
#endif
    gsArgs.isPanelMirrored = orientation==PanelOrientation::Reflected;
    pWaterPoloPanel->setMirrored(gsArgs.isPanelMirrored);
}


void
WaterPoloCtrl::processBtMessage(QString sMessage) {
    QString sToken;
    bool ok;
    int iTeam;
    QString sNoData = QString("NoData");

    sToken = XML_Parse(sMessage, "startT");
    if(sToken != sNoData) {
        onCountStart(0);
    }// start Time

    sToken = XML_Parse(sMessage, "stopT");
    if(sToken != sNoData) {
        onCountStop(0);
    }// start Time

    sToken = XML_Parse(sMessage, "team0");
    if(sToken != sNoData) {
        onTeamTextChanged(sToken.left(maxTeamNameLen), 0);
    }// team 0 name

    sToken = XML_Parse(sMessage, "team1");
    if(sToken != sNoData){
        onTeamTextChanged(sToken.left(maxTeamNameLen), 1);
    }// team 1 name

    sToken = XML_Parse(sMessage, "inctimeout");
    if(sToken != sNoData){
        iTeam = sToken.toInt(&ok);
        if(!ok || (iTeam<0) || (iTeam>1))
            return;
        onTimeOutIncrement(iTeam);
    }// increment timeout

    sToken = XML_Parse(sMessage, "dectimeout");
    if(sToken != sNoData){
        iTeam = sToken.toInt(&ok);
        if(!ok || (iTeam<0) || (iTeam>1))
            return;
        onTimeOutDecrement(iTeam);
    }// decrement timeout

    sToken = XML_Parse(sMessage, "incscore");
    if(sToken != sNoData) {
        iTeam = sToken.toInt(&ok);
        if(!ok || (iTeam<0) || (iTeam>1))
            return;
        onScoreIncrement(iTeam);
    }// increment score

    sToken = XML_Parse(sMessage, "decscore");
    if(sToken != sNoData) {
        iTeam = sToken.toInt(&ok);
        if(!ok || (iTeam<0) || (iTeam>1))
            return;
        onScoreDecrement(iTeam);
    }// decrement score

    sToken = XML_Parse(sMessage, "setOrientation");
    if(sToken != sNoData) {
        PanelOrientation orientation = PanelOrientation(sToken.toInt(&ok));
        if(!ok)
            return;
        onChangePanelOrientation(orientation);
    }// mirrored

    sToken = XML_Parse(sMessage, "startspotloop");
    if(sToken != sNoData) {
        onButtonSpotLoopClicked();
    }// startSpotLoop

    sToken = XML_Parse(sMessage, "endspotloop");
    if(sToken != sNoData) {
        onButtonSpotLoopClicked();
    }// stop spotloop

    sToken = XML_Parse(sMessage, "startslideshow");
    if(sToken != sNoData) {
        onButtonSlideShowClicked();
    }// start slideshow

    sToken = XML_Parse(sMessage, "endslideshow");
    if(sToken != sNoData) {
        onButtonSlideShowClicked();
    }// stop slideshow

    sToken = XML_Parse(sMessage, "fieldExchange");
    if(sToken != sNoData) {
        exchangeField();
    }// Change Field

    sToken = XML_Parse(sMessage, "period");
    if(sToken != sNoData) {
        startNewPeriod();
    }// Change Field

    sToken = XML_Parse(sMessage, "newTime");
    if(sToken != sNoData) {
        QRegularExpression rx("[:]");// match a colon
        QStringList list = sToken.split(rx, Qt::SkipEmptyParts);
        if(list.count() < 2) return;
        int minutes = list[0].toInt();
        int seconds = list[1].toInt();
        qint64 mSecToGo = (minutes*60+seconds)*1000;
        if(mSecToGo <= gsArgs.iTimeDuration*60000) {
            runMilliSeconds = gsArgs.iTimeDuration * 60000-mSecToGo;
            if(runMilliSeconds >= 0) {
                QString sRemainingTime = QString("%1:%2")
                                             .arg(minutes, 1)
                                             .arg(seconds, 2, 10, QChar('0'));
                pTimeEdit->setText(sRemainingTime);
                pWaterPoloPanel->setTime(sRemainingTime);
                pCountStart->setEnabled(true);
                QString sMessage = QString("<time>%1</time>")
                                       .arg(sRemainingTime);
                pBtServer->sendMessage(sMessage);
            }
        }
    }// Change Remaining Time
}


