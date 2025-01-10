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
#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QBluetoothHostInfo>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothservicediscoveryagent.h>
#include <QNetworkInterface>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif


///////////////////////////////////////////////////////////////////////
// In Android pare non funzionare il QBluetoothServiceDiscoveryAgent //
///////////////////////////////////////////////////////////////////////


#include "btscorecontroller.h"
#include "utility.h"
#include "btclient.h"

#if QT_FEATURE_permissions
#include <QtCore/qcoreapplication.h>
#include <QtCore/qpermissions.h>
#endif
#include <QtWidgets/qmessagebox.h>


using namespace Qt::StringLiterals;
static constexpr auto serviceUuid = "aacf3e05-6531-43f3-9fdc-f0e3b3531f0c"_L1;


BtScoreController::BtScoreController(QFile *myLogFile, QWidget *parent)
    : QMainWindow(parent)
    , pLogFile(myLogFile)
    , pSettings(new QSettings("Gabriele Salvato", "Score Controller"))
    , pPanelClient(nullptr)
    , pBtDiscoveryAgent(nullptr)
    , pAdapter(nullptr)
    , pairedDevices(QStringList())
{
    qApp->installEventFilter(this);
    setWindowTitle("Score Controller - ©Gabriele Salvato (2025)");
    setWindowIcon(QIcon(":/CommonFiles/Logo.ico"));

    pSpotButtonsLayout = CreateSpotButtons();
    connectButtonSignals();

    initBluetooth();

    currentDevice = -1;
#ifdef Q_OS_ANDROID
    pairedDevices.append(getBluetoothDevicesAdress());
#elif defined Q_OS_LINUX
    pairedDevices.append(getBluetoothDevicesAdress());
#endif

#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
    for(int i=0; i<pairedDevices.count(); i++)
        qCritical() << "Paired Device:" << pairedDevices.at(i);
#endif

    // Let's try first the saved address (if any)
    QBluetoothAddress address(QBluetoothAddress(pSettings->value("ServerAddress", "").toString()));
    if(!address.isNull()) {
        tryConnectLastKnown(address);
    }
    else { // else We will try to either:
           //   connect to Paired devices (LINUX or ANDROID)
           //   or using QBluetoothServiceDiscoveryAgent (WINDOWS)
#ifdef Q_OS_ANDROID
        tryPaired();
#elif defined Q_OS_LINUX
        tryPaired();
#else
        startBtDiscovery(QBluetoothUuid(serviceUuid));
#endif
    }

    setDisabled(true);
    myStatus = showPanel;
}


BtScoreController::~BtScoreController() {
    if(pBtDiscoveryAgent)
        delete pBtDiscoveryAgent;
    pBtDiscoveryAgent = nullptr;
    if(pSettings) delete pSettings;
    pSettings = Q_NULLPTR;
    if(pPanelClient) {
        pPanelClient->disconnect();
        delete pPanelClient;
    }
    pPanelClient = nullptr;
}


QStringList
BtScoreController::getBluetoothDevicesAdress() {
    QStringList result;
    // QString fmt("%1 %2");
    QString fmt("%1");
#ifdef Q_OS_ANDROID
    // Query via Android Java API.
    QJniObject adapter = QJniObject::callStaticObjectMethod("android/bluetooth/BluetoothAdapter",
                                                            "getDefaultAdapter",
                                                            "()Landroid/bluetooth/BluetoothAdapter;"); // returns a BluetoothAdapter
    if (checkException("BluetoothAdapter.getDefaultAdapter()", &adapter)) {
        return result;
    }
    QJniObject pairedDevicesSet=adapter.callObjectMethod("getBondedDevices",
                                                         "()Ljava/util/Set;"); // returns a Set<BluetoothDevice>
    if (checkException("BluetoothAdapter.getBondedDevices()", &pairedDevicesSet)) {
        return result;
    }
    jint size = pairedDevicesSet.callMethod<jint>("size");
    checkException("Set<BluetoothDevice>.size()", &pairedDevicesSet);
    if (size>0) {
        QJniObject iterator=pairedDevicesSet.callObjectMethod("iterator",
                                                              "()Ljava/util/Iterator;"); // returns an Iterator<BluetoothDevice>
        if (checkException("Set<BluetoothDevice>.iterator()", &iterator)) {
            return result;
        }
        for (int i=0; i<size; i++) {
            QJniObject dev=iterator.callObjectMethod("next",
                                                     "()Ljava/lang/Object;"); // returns a BluetoothDevice
            if (checkException("Iterator<BluetoothDevice>.next()", &dev)) {
                continue;
            }
            QString address=dev.callObjectMethod("getAddress",
                                                 "()Ljava/lang/String;").toString(); // returns a String
            QString name=dev.callObjectMethod("getName",
                                              "()Ljava/lang/String;").toString(); // returns a String
            // result.append(fmt.arg(address).arg(name));
            result.append(fmt.arg(address));
        }
    }
#elif defined Q_OS_LINUX
    // Query via the Linux command bluetoothctl devices.
    QProcess command;
    command.start("/usr/bin/bluetoothctl",  QStringList("devices"));
    command.waitForFinished(3000);
    if (command.error()==QProcess::FailedToStart) {
        qWarning("Cannot execute the command '/usr/bin/bluetoothctl': %s",qPrintable(command.errorString()));
    }
    else {
        // Parse the output, example: HC-06 (20:13:11:15:16:08)
        QByteArray output=command.readAllStandardOutput();
        foreach(QByteArray line, output.split('\n')) {
            QList<QByteArray> elements =line.split(' ');
             if (elements.count() > 1) {
                result.append(fmt.arg(elements.at(1)));
#ifdef BT_DEBUG
                qCritical() << elements.at(1) << elements.at(2);
#endif
            }
        }
    }
#endif
    return result;
}


#ifdef Q_OS_ANDROID
bool
BtScoreController::checkException(const char* method, const QJniObject* obj) {
    static QJniEnvironment env;
    bool result=false;
    if (env->ExceptionCheck()) {
        qCritical("Exception in %s",method);
        env->ExceptionDescribe();
        env->ExceptionClear();
        result=true;
    }
    if (!(obj==NULL || obj->isValid())) {
        qCritical("Invalid object returned by %s",method);
        result=true;
    }
    return result;
}
#endif


void
BtScoreController::closeEvent(QCloseEvent*) {
    if(pBtDiscoveryAgent)
        delete pBtDiscoveryAgent;
    pBtDiscoveryAgent = nullptr;
    if(pSettings) delete pSettings;
    pSettings = Q_NULLPTR;
    if(pPanelClient) {
        pPanelClient->disconnect();
        delete pPanelClient;
    }
    pPanelClient = nullptr;
}


QHBoxLayout*
BtScoreController::CreateSpotButtons() {
    auto* spotButtonLayout = new QHBoxLayout();

    QPixmap pixmap(":/CommonFiles/ButtonIcons/PlaySpots.png");
    QIcon ButtonIcon(pixmap);
    pSpotButton = new QPushButton(ButtonIcon, "");
    pSpotButton->setIconSize(pixmap.rect().size());
    pSpotButton->setFlat(true);
    pSpotButton->setToolTip("Start/Stop Spot Loop");

    pixmap.load(":/CommonFiles/ButtonIcons/PlaySlides.png");
    ButtonIcon.addPixmap(pixmap);
    pSlideShowButton = new QPushButton(ButtonIcon, "");
    pSlideShowButton->setIconSize(pixmap.rect().size());
    pSlideShowButton->setFlat(true);
    pSlideShowButton->setToolTip("Start/Stop Slide Show");

    pixmap.load(":/CommonFiles/ButtonIcons/PanelSetup.png");
    ButtonIcon.addPixmap(pixmap);
    pGeneralSetupButton = new QPushButton(ButtonIcon, "");
    pGeneralSetupButton->setIconSize(pixmap.rect().size());
    pGeneralSetupButton->setFlat(true);
    pGeneralSetupButton->setToolTip("General Setup");

    pixmap.load(":/CommonFiles/ButtonIcons/Off.png");
    ButtonIcon.addPixmap(pixmap);
    pSwitchOffButton = new QPushButton(ButtonIcon, "");
    pSwitchOffButton->setIconSize(pixmap.rect().size());
    pSwitchOffButton->setFlat(true);
    pSwitchOffButton->setToolTip("Switch Panel Off");

    spotButtonLayout->addWidget(pSpotButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pSlideShowButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pGeneralSetupButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pSwitchOffButton);
    spotButtonLayout->addStretch();

    return spotButtonLayout;
}


void
BtScoreController::connectButtonSignals() {
    connect(pSpotButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSpotLoopClicked()));
    connect(pSlideShowButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSlideShowClicked()));
    connect(pGeneralSetupButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSetupClicked()));
    connect(pSwitchOffButton, SIGNAL(clicked(bool)),
            this, SLOT(onOffButtonClicked()));

}


// Dummy... see Volley Panel
void
BtScoreController::GeneralSetup() {
}


void
BtScoreController::initBluetooth() {
#if QT_FEATURE_permissions
    QBluetoothPermission permission{};
    switch (qApp->checkPermission(permission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(permission, this, &BtScoreController::initBluetooth);
        return;
    case Qt::PermissionStatus::Denied:
        QMessageBox::warning(this, tr("Missing permissions"),
                             tr("Permissions are needed to use Bluetooth. "
                                "Please grant the permissions to this "
                                "application in the system settings."));
        qApp->quit();
        return;
    case Qt::PermissionStatus::Granted:
        break; // proceed to initialization
    }
#endif // QT_CONFIG(permissions)

    localAdapters = QBluetoothLocalDevice::allDevices();
    if(localAdapters.isEmpty()) {
        qCritical("Bluetooth adapter not found! The application can't work.");
        exit(EXIT_FAILURE);
    }
    pAdapter = new QBluetoothLocalDevice(localAdapters.at(0).address());
    pAdapter->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    sLocalName = pAdapter->name();

    pBtDiscoveryAgent = new QBluetoothServiceDiscoveryAgent(pAdapter);

    connect(pBtDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
            this, &BtScoreController::serviceDiscovered);
    connect(pBtDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,
            this, &BtScoreController::discoveryFinished);
    connect(pBtDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled,
            this, &BtScoreController::discoveryFinished);
}


int
BtScoreController::sendMessage(const QString& sMessage) {
    if(pPanelClient)
        pPanelClient->sendMessage(sMessage);
    return 0;
}


void
BtScoreController::tryConnectLastKnown(QBluetoothAddress address) {
    QBluetoothUuid uuid(serviceUuid);
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
    qCritical() << "BtAddress:" << address.toString();
    qCritical() << "BtUUID   :" << uuid.toString();
#endif
    pPanelClient = new BtClient(this);
    pPanelClient->startClient(address, uuid);
    connect(pPanelClient, SIGNAL(connected(QString)),
            this, SLOT(onPanelClientConnected(QString)));
    connect(pPanelClient, SIGNAL(socketErrorOccurred(QString)),
            this, SLOT(onPanelClientSocketError(QString)));
}


void
BtScoreController::tryPaired() {
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
#endif
    if(pPanelClient) {
        pPanelClient->disconnect();
        pPanelClient->deleteLater();
        pPanelClient = nullptr;
    }
    currentDevice = (currentDevice+1) % pairedDevices.count();
    QBluetoothServiceInfo serviceInfo;
    QBluetoothAddress address(pairedDevices.at(currentDevice));
    QBluetoothUuid uuid(serviceUuid);
    pPanelClient = new BtClient(this);
    pPanelClient->startClient(address, uuid);
    connect(pPanelClient, SIGNAL(connected(QString)),
            this, SLOT(onPanelClientConnected(QString)));
    connect(pPanelClient, SIGNAL(socketErrorOccurred(QString)),
            this, SLOT(onPanelClientSocketError(QString)));
}


/*
void
BtScoreController::try2ConnectBt() {
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
#endif
    if(pPanelClient) {
        pPanelClient->disconnect();
        pPanelClient->deleteLater();
        pPanelClient = nullptr;
    }
    // QBluetoothServiceInfo serviceInfo;
    // QBluetoothAddress address;
    // if(!pairedDevices.isEmpty())
    //     address = QBluetoothAddress(pairedDevices.at(currentDevice));
    // else
    //     address = QBluetoothAddress(pSettings->value("ServerAddress", "").toString());
    // QBluetoothUuid uuid(serviceUuid);
// #ifdef BT_DEBUG
//     qCritical() << "BtAddress:" << address.toString();
//     qCritical() << "BtUUID   :" << uuid.toString();
// #endif
    // if((!address.isNull()) && (!uuid.isNull())) {
    //     pTempClient = new BtClient(this);
    //     pTempClient->startClient(address, uuid);
    //     connect(pTempClient, SIGNAL(connected(QString)),
    //             this, SLOT(onPanelClientConnected(QString)));
    //     if(!pairedDevices.isEmpty()) {
    //         connect(pTempClient, SIGNAL(socketErrorOccurred(QString)),
    //                 this, SLOT(onPanelClientSocketError(QString)));
    //     }
    // }
    startBtDiscovery(QBluetoothUuid(serviceUuid));
}
*/

void
BtScoreController::SaveStatus() {

}


void
BtScoreController::disableGeneralButtons() {
    pSpotButton->setDisabled(true);
    pSlideShowButton->setDisabled(true);
    pSwitchOffButton->setDisabled(true);
    pGeneralSetupButton->setDisabled(true);
}


void
BtScoreController::enableGeneralButtons() {
    pSpotButton->setEnabled(true);
    pSlideShowButton->setEnabled(true);
    pSwitchOffButton->setEnabled(true);
    pGeneralSetupButton->setEnabled(true);
}



void
BtScoreController::onButtonSetupClicked() {
    GeneralSetup();
}


void
BtScoreController::onButtonSpotLoopClicked() {
    QString sMessage;
    if(myStatus == showPanel) {
        sMessage = QString("<startspotloop>1</startspotloop>");
        sendMessage(sMessage);
    }
    else {
        sMessage = "<endspotloop>1</endspotloop>";
        sendMessage(sMessage);
    }
}


void
BtScoreController::onButtonSlideShowClicked() {
    QString sMessage;
    if(myStatus == showPanel) {
        sMessage = "<startslideshow>1</startslideshow>";
        sendMessage(sMessage);
    }
    else {
        sMessage = "<endslideshow>1</endslideshow>";
        sendMessage(sMessage);
    }
}


void
BtScoreController::onOffButtonClicked() {
    int iRes = QMessageBox::question(this, tr("Score_Controller"),
                                     tr("Vuoi Davvero Spegnere il Segnapunti ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;
    QString sMessage;
    sMessage = "<kill>1</kill>";
    sendMessage(sMessage);
}


/////////////////////////////////////////////////////////////
/////////////////// Bluetooth Service Discovery Management //
/////////////////////////////////////////////////////////////

void
BtScoreController::startBtDiscovery(const QBluetoothUuid &uuid) {
    if (pBtDiscoveryAgent->isActive())
        pBtDiscoveryAgent->stop();
    pBtDiscoveryAgent->setUuidFilter(uuid);
    pBtDiscoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
    qCritical() << "Bluetooth Discovery Started";
#endif
}


void
BtScoreController::stopBtDiscovery() {
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
#endif
    if (pBtDiscoveryAgent) {
        pBtDiscoveryAgent->stop();
    }
}


void
BtScoreController::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo) {
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
    qCritical() << "ServerAddress" << serviceInfo.device().address().toString();
    qCritical() << "ServerUUID" << serviceInfo.serviceUuid().toString();
#endif
    pBtDiscoveryAgent->disconnect();
    pBtDiscoveryAgent->stop();
    pSettings->setValue("ServerAddress", serviceInfo.device().address().toString());
    pSettings->setValue("ServerUUID", serviceInfo.serviceUuid().toString());
    const QBluetoothAddress address = serviceInfo.device().address();
    QString remoteName;
    if (serviceInfo.device().name().isEmpty())
        remoteName = address.toString();
    else
        remoteName = serviceInfo.device().name();
    if(pPanelClient) {
        pPanelClient->disconnect();
        delete pPanelClient;
    }
    pPanelClient = new BtClient(this);
    connect(pPanelClient, SIGNAL(connected(QString)),
            this, SLOT(onPanelClientConnected(QString)));
    pPanelClient->startClient(serviceInfo);
}


void
BtScoreController::discoveryFinished() {
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
    qCritical() << "Error:" << pBtDiscoveryAgent->errorString();
#endif
    if(!pPanelClient)
        startBtDiscovery(QBluetoothUuid(serviceUuid));
}


void
BtScoreController::onTextMessageReceived(const QString sSource, const QString sMessage) {
    Q_UNUSED(sSource)
    processTextMessage(sMessage);
}


void
BtScoreController::processTextMessage(QString sMessage) {
    Q_UNUSED(sMessage)
}


void
BtScoreController::processGeneralMessages(QString sMessage) {
    QString sToken;
    QString sNoData = QString("NoData");
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
    qCritical() << sMessage;
#endif
    sToken = XML_Parse(sMessage, "startSpotLoop");
    if(sToken != sNoData) {
        QPixmap pixmap(":/CommonFiles/ButtonIcons/sign_stop.png");
        QIcon ButtonIcon(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setDisabled(true);
        pGeneralSetupButton->setDisabled(true);
        myStatus = showSpots;
    }

    sToken = XML_Parse(sMessage, "endSpotLoop");
    if(sToken != sNoData) {
        QPixmap pixmap(":/CommonFiles/ButtonIcons/PlaySpots.png");
        QIcon ButtonIcon(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
        myStatus = showPanel;
    }

    sToken = XML_Parse(sMessage, "startSlideShow");
    if(sToken != sNoData) {
        QPixmap pixmap(":/CommonFiles/ButtonIcons/sign_stop.png");
        QIcon ButtonIcon(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        pSpotButton->setDisabled(true);
        pGeneralSetupButton->setDisabled(true);
        myStatus = showSpots;
    }

    sToken = XML_Parse(sMessage, "endSlideShow");
    if(sToken != sNoData) {
        QPixmap pixmap(":/CommonFiles/ButtonIcons/PlaySlides.png");
        QIcon ButtonIcon(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        pSpotButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
        myStatus = showPanel;
    }
}


void
BtScoreController::onPanelClientConnected(QString sName) {
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
    qCritical() << "Connected to" << sName;
#endif
    Q_UNUSED(sName)
    stopBtDiscovery();
    setEnabled(true);
    connect(pPanelClient, SIGNAL(disconnected()),
            this, SLOT(onPanelClientDisconnected()));
    connect(pPanelClient, SIGNAL(socketErrorOccurred(QString)),
            this, SLOT(onPanelClientSocketError(QString)));
    connect(pPanelClient, SIGNAL(messageReceived(QString,QString)),
            this, SLOT(onTextMessageReceived(QString,QString)));
}


void
BtScoreController::onPanelClientDisconnected() {
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
#endif
    setDisabled(true);
    if(pPanelClient) {
        pPanelClient->disconnect();
        pPanelClient->deleteLater();
        pPanelClient = nullptr;
    }
#ifdef Q_OS_ANDROID
    tryPaired();
#elif defined Q_OS_LINUX
    tryPaired();
#else
    startBtDiscovery(QBluetoothUuid(serviceUuid));
#endif
}


void
BtScoreController::onPanelClientSocketError(QString sError) {
#ifdef BT_DEBUG
    qCritical() << __FUNCTION__ << __LINE__;
#endif
    Q_UNUSED(sError)
    setDisabled(true);
    if(pPanelClient) {
        pPanelClient->disconnect();
        pPanelClient->deleteLater();
        pPanelClient = nullptr;
    }
#ifdef Q_OS_ANDROID
    tryPaired();
#elif defined Q_OS_LINUX
    tryPaired();
#else
    startBtDiscovery(QBluetoothUuid(serviceUuid));
#endif
}
