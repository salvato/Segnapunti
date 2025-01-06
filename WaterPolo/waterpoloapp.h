#pragma once

#include <QApplication>
#include <QObject>
#include <QTimer>


QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(WaterPoloCtrl)
QT_FORWARD_DECLARE_CLASS(QFile)


class WaterPoloApp : public QApplication
{
    Q_OBJECT
public:
    WaterPoloApp(int& argc, char** argv);

private:
    bool PrepareLogFile();

private:
    QSettings*        pSettings;
    QFile*            pLogFile;
    WaterPoloCtrl*    pScoreController;
    QString           sLanguage;
    QString           logFileName;
};

