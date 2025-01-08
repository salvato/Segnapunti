#include "waterpoloapplication.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WaterpoloApplication w;
    w.show();
    return a.exec();
}
