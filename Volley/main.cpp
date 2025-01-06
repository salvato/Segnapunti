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
#include "volleyapplication.h"

#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QSurfaceFormat>

int
main(int argc, char *argv[]) {

    // qputenv("QT_LOGGING_RULES","*.debug=false;qt.qpa.*=true");
    // qputenv("QT_LOGGING_RULES","*.debug=false;qt.qpa.*=false"); // supress anoying messages
    // qputenv("QSG_INFO", "1");
    // qputenv("QT_FATAL_WARNINGS", "1");

    QSurfaceFormat format;
#ifdef Q_PROCESSOR_ARM_64
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(3, 1);
#endif
    format.setDepthBufferSize(32);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    VolleyApplication a(argc, argv);
    QString sVersion = QString("3.00");
    a.setApplicationVersion(sVersion);

    int iResult = a.exec();
    return iResult;
}
