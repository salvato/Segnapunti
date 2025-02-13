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

#include <QApplication>
#include <QTranslator>
#include <QTimer>


QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(VolleyController)
QT_FORWARD_DECLARE_CLASS(VolleyPanel)
QT_FORWARD_DECLARE_CLASS(QFile)


class VolleyApplication : public QApplication
{
    Q_OBJECT
public:
    VolleyApplication(int& argc, char** argv);

private:
    bool PrepareLogFile();

private:
    QSettings*        pSettings;
    QFile*            pLogFile;
    VolleyController* pScoreController;
    QString           logFileName;
};
