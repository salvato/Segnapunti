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

#include <QObject>
#include <QLabel>

#include "../CommonFiles/scorepanel.h"

class WaterPoloPanel : public ScorePanel
{
public:
    WaterPoloPanel(QFile *myLogFile, QWidget *parent = nullptr);
    ~WaterPoloPanel();
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void setTeam(int iTeam, QString sTeamName);
    void setScore(int iTeam, int iScore);
    void setTime(QString sTime);
    void setPeriod(int iPeriod);
    void setTimeout(int iTeam, int iTimeout);
    void setMirrored(bool isPanelMirrored);
    bool getMirrored();
    void setLogo(int iTeam, QString sFileLogo);

private:
    void         createPanelElements();
    QGridLayout* createPanel();

private:
    QLabel*           pTeam[2];
    QLabel*           pScore[2];
    QLabel*           pScoreLabel;
    QLabel*           pTimeLabel;
    QLabel*           pPeriodLabel;
    QLabel*           pPeriod;
    // QLabel*           pTimeout[2];
    // QLabel*           pTimeoutLabel;
    QLabel*           logoLabel[2];
    QLabel*           pCopyRight;
    QPixmap*          pPixmapLogo[2];

    QString           sFontName;
    int               fontWeight;
    QPalette          panelPalette;
    QLinearGradient   panelGradient;
    QBrush            panelBrush;
    int               iServizio;
    // int               iTimeoutFontSize;
    int               iScoreFontSize;
    int               iTimeFontSize;
    int               iTeamFontSize;
    int               iLabelsFontSize;
    int               iLogoSize;
    int               maxTeamNameLen;
};
