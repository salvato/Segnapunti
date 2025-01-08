#include "waterpoloapplication.h"

#include <QApplication>

int
main(int argc, char *argv[]) {
    WaterpoloApplication a(argc, argv);
    QString sVersion = QString("3.00");
    a.setApplicationVersion(sVersion);
    int iResult = a.exec();
    return iResult;
}
