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
#include "waterpolopanel.h"
#include "../CommonFiles/utility.h"

#include <QScreen>
#include <QProcess>
#include <QCloseEvent>
#include <QGridLayout>
#include <QGuiApplication>


WaterPoloPanel::WaterPoloPanel(QFile *myLogFile, QWidget *parent)
    : ScorePanel(myLogFile, parent)
    , iServizio(0)
    , maxTeamNameLen(15)
{
    pPixmapLogo[0] = new QPixmap(QString(":/../CommonFiles/Logo_UniMe.png"));
    pPixmapLogo[1] = new QPixmap(QString(":/../CommonFiles/Logo_SSD_UniMe.png"));

    sFontName = QString("Liberation Sans Bold");
    fontWeight = QFont::Black;

    // Matrice di 9 Righe x 12 Colonne
    QSize panelSize = QGuiApplication::primaryScreen()->geometry().size();
    int iRowSize = panelSize.height()/9;
    iTeamFontSize    = std::min(iRowSize,
                                int(panelSize.width()/(2.2*maxTeamNameLen)));
    iScoreFontSize   = std::min(3*iRowSize,
                                int(panelSize.width()/9));
    iTimeFontSize    = std::min(5*iRowSize,
                                int(panelSize.width()/6));
    iLabelsFontSize  = iRowSize;
    iLogoSize = iTimeFontSize;
    // iTimeoutFontSize =iTeamFontSize

    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
    panelGradient.setColorAt(0, QColor(0, 0, START_GRADIENT));
    panelGradient.setColorAt(1, QColor(0, 0, END_GRADIENT));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active,   QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);
    panelPalette.setColor(QPalette::WindowText,    Qt::yellow);
    panelPalette.setColor(QPalette::Base,          Qt::black);
    panelPalette.setColor(QPalette::AlternateBase, Qt::blue);
    panelPalette.setColor(QPalette::Text,          Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,    Qt::white);
    setPalette(panelPalette);

    createPanelElements();
    buildLayout();
}


WaterPoloPanel::~WaterPoloPanel() {
}


void
WaterPoloPanel::setTeam(int iTeam, QString sTeamName) {
    pTeam[iTeam]->setText(sTeamName.left(maxTeamNameLen));
}


void
WaterPoloPanel::setScore(int iTeam, int iScore) {
    pScore[iTeam]->setText(QString("%1").arg(iScore));
}


void
WaterPoloPanel::setTime(QString sTime) {
    pTimeLabel->setText(sTime);
}


void
WaterPoloPanel::setPeriod(int iPeriod) {
    pPeriod->setText(QString("%1").arg(iPeriod));
}


// void
// WaterPoloPanel::setTimeout(int iTeam, int iTimeout) {
//     pTimeout[iTeam]->setText(QString("%1").arg(iTimeout));
// }


void
WaterPoloPanel::setMirrored(bool isPanelMirrored) {
    isMirrored = isPanelMirrored;
    buildLayout();
}


bool
WaterPoloPanel::getMirrored() {
    return isMirrored;
}


void
WaterPoloPanel::closeEvent(QCloseEvent *event) {
    ScorePanel::closeEvent(event);
    event->accept();
}


void
WaterPoloPanel::createPanelElements() {
    // Timeout
    // pTimeoutLabel = new QLabel("Timeout");
    // pTimeoutLabel->setFont(QFont(sFontName, iLabelsFontSize/2, fontWeight));
    // pTimeoutLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // for(int i=0; i<2; i++) {
    //     pTimeout[i] = new QLabel("8");
    //     pTimeout[i]->setFrameStyle(QFrame::NoFrame);
    //     pTimeout[i]->setFont(QFont(sFontName, iTimeoutFontSize, fontWeight));
    // }

    // Score
    for(int i=0; i<2; i++){
        pScore[i] = new QLabel("88");
        pScore[i]->setAlignment(Qt::AlignHCenter);
        pScore[i]->setFont(QFont(sFontName, iScoreFontSize, fontWeight));
        pScore[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    // Period
    pPeriodLabel = new QLabel(tr("Periodo"));
    pPeriodLabel->setFont(QFont(sFontName, iLabelsFontSize/3, fontWeight));
    pPeriodLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pPeriod = new QLabel("1");
    pPeriod->setAlignment(Qt::AlignHCenter);
    pPeriod->setFont(QFont(sFontName, iLabelsFontSize, fontWeight));
    pPeriod->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Time
    pTimeLabel = new QLabel(tr("0:00"));
    pTimeLabel->setAlignment(Qt::AlignHCenter);
    pTimeLabel->setFont(QFont(sFontName, iTimeFontSize, fontWeight));
    pTimeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Loghi
    for(int i=0; i<2; i++){
        logoLabel[i] = new QLabel(" ");
        logoLabel[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    // Teams
    QPalette pal = panelPalette;
    pal.setColor(QPalette::WindowText, Qt::white);
    for(int i=0; i<2; i++) {
        pTeam[i] = new QLabel();
        pTeam[i]->setPalette(pal);
        pTeam[i]->setFont(QFont(sFontName, iTeamFontSize, fontWeight));
        pTeam[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    pTeam[0]->setText(tr("Locali"));
    pTeam[1]->setText(tr("Ospiti"));

    // Copyright
    pCopyRight = new QLabel("© Gabriele Salvato (2025)");
    pCopyRight->setPalette(pal);
    pCopyRight->setFont(QFont(sFontName, iTeamFontSize/2, fontWeight));
    pCopyRight->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


QGridLayout*
WaterPoloPanel::createPanel() {
    QGridLayout *layout = new QGridLayout(); // 9 Righe x 12 Colonne

    int ileft  = 0;
    int iright = 1;
    if(isMirrored) {
        ileft  = 1;
        iright = 0;
    }

    layout->addWidget(pTeam[ileft],      0, 0, 1, 6, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(pTeam[iright],     0, 6, 1, 6, Qt::AlignHCenter|Qt::AlignVCenter);

    layout->addWidget(pScore[ileft],     1, 1, 3, 3, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(pPeriodLabel,      1, 4, 1, 4, Qt::AlignHCenter|Qt::AlignBottom);
    layout->addWidget(pScore[iright],    1, 8, 3, 3, Qt::AlignHCenter|Qt::AlignVCenter);

    layout->addWidget(pPeriod,           2, 4, 1, 4,Qt::AlignHCenter|Qt::AlignTop);

    layout->addWidget(logoLabel[ileft],  4, 0, 4, 4, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(pTimeLabel,        4, 4, 4, 4, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(logoLabel[iright], 4, 8, 4, 4, Qt::AlignHCenter|Qt::AlignVCenter);

    // layout->addWidget(pTimeout[ileft],   10, 3, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
    // layout->addWidget(pTimeoutLabel,     10, 4, 2, 4, Qt::AlignHCenter|Qt::AlignVCenter);
    // layout->addWidget(pTimeout[iright],  10, 8, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);

    layout->addWidget(pCopyRight,        8, 6, 1, 6, Qt::AlignRight|Qt::AlignVCenter);

    return layout;
}


void
WaterPoloPanel::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("%1  %2")
                   .arg(pSetLabel->text(), pScoreLabel->text()));
#endif
        pScoreLabel->setText(tr("Punti"));
    } else
        QWidget::changeEvent(event);
}


void
WaterPoloPanel::setLogo(int iTeam, QString sFileLogo) {
    if(QFile::exists(sFileLogo)) {
        if(pPixmapLogo[iTeam]) delete pPixmapLogo[iTeam];
        pPixmapLogo[iTeam] =  new QPixmap(sFileLogo);
        logoLabel[iTeam]->setPixmap(pPixmapLogo[iTeam]->scaled(iLogoSize, iLogoSize, Qt::KeepAspectRatio));
    }
}


