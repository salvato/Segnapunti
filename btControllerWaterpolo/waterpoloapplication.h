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

#include <QApplication>


QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(WaterpoloController)
QT_FORWARD_DECLARE_CLASS(QFile)


class WaterpoloApplication : public QApplication
{
    Q_OBJECT

public:
    WaterpoloApplication(int& argc, char** argvr);
    ~WaterpoloApplication();

private:
    bool PrepareLogFile();

private:
    QSettings*           pSettings;
    QFile*               pLogFile;
    WaterpoloController* pScoreController;
    QString              sLanguage;
    QString              logFileName;
};
