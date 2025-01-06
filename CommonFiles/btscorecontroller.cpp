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


#include "btscorecontroller.h"
#include "../CommonFiles/utility.h"
#include "btclient.h"

#if QT_FEATURE_permissions
#include <QtCore/qcoreapplication.h>
#include <QtCore/qpermissions.h>
#include <QtWidgets/qmessagebox.h>
#endif


using namespace Qt::StringLiterals;
static constexpr auto serviceUuid = "aacf3e05-6531-43f3-9fdc-f0e3b3531f0c"_L1;


BtScoreController::BtScoreController(QFile *myLogFile, QWidget *parent)
    : QMainWindow(parent)
    , pLogFile(myLogFile)
    , pSettings(new QSettings("Gabriele Salvato", "Score Controller"))
    , pTempClient(nullptr)
    , pPanelClient(nullptr)
    , pBtDiscoveryAgent(nullptr)
    , pAdapter(nullptr)
{
    qApp->installEventFilter(this);
    setWindowTitle("Score Controller - ©Gabriele Salvato (2025)");
    setWindowIcon(QIcon(":/Logo.ico"));

    pSpotButtonsLayout = CreateSpotButtons();
    connectButtonSignals();

    initBluetooth();
    try2ConnectBt();

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

    QPixmap pixmap(":/ButtonIcons/PlaySpots.png");
    QIcon ButtonIcon(pixmap);
    pSpotButton = new QPushButton(ButtonIcon, "");
    pSpotButton->setIconSize(pixmap.rect().size());
    pSpotButton->setFlat(true);
    pSpotButton->setToolTip("Start/Stop Spot Loop");

    pixmap.load(":/ButtonIcons/PlaySlides.png");
    ButtonIcon.addPixmap(pixmap);
    pSlideShowButton = new QPushButton(ButtonIcon, "");
    pSlideShowButton->setIconSize(pixmap.rect().size());
    pSlideShowButton->setFlat(true);
    pSlideShowButton->setToolTip("Start/Stop Slide Show");

    pixmap.load(":/ButtonIcons/PanelSetup.png");
    ButtonIcon.addPixmap(pixmap);
    pGeneralSetupButton = new QPushButton(ButtonIcon, "");
    pGeneralSetupButton->setIconSize(pixmap.rect().size());
    pGeneralSetupButton->setFlat(true);
    pGeneralSetupButton->setToolTip("General Setup");

    pixmap.load(":/ButtonIcons/Off.png");
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
BtScoreController::try2ConnectBt() {
    if(pTempClient) {
        pTempClient->disconnect();
        pTempClient->deleteLater();
        pTempClient = nullptr;
    }
    QBluetoothServiceInfo serviceInfo;
    QBluetoothAddress address(pSettings->value("ServerAddress", "").toString());
    QBluetoothUuid uuid(pSettings->value("ServerUUID", "").toString());
    qCritical() << "BtAddress:" << address.toString();
    qCritical() << "BtUUID   :" << uuid.toString();
    if((!address.isNull()) && (!uuid.isNull())) {
        pTempClient = new BtClient(this);
        pTempClient->startClient(address, uuid);
        connect(pTempClient, SIGNAL(connected(QString)),
                this, SLOT(onPanelClientConnected(QString)));
    }
    startBtDiscovery(QBluetoothUuid(serviceUuid));
}


void
BtScoreController::SaveStatus() {

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
    // qCritical() << "Bluetooth Discovery Started";
}


void
BtScoreController::stopBtDiscovery() {
    if (pBtDiscoveryAgent) {
        pBtDiscoveryAgent->stop();
    }
}


void
BtScoreController::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo) {
    pSettings->setValue("ServerAddress", serviceInfo.device().address().toString());
    pSettings->setValue("ServerUUID", serviceInfo.serviceUuid().toString());

    const QBluetoothAddress address = serviceInfo.device().address();
    QString remoteName;
    if (serviceInfo.device().name().isEmpty())
        remoteName = address.toString();
    else
        remoteName = serviceInfo.device().name();
    if(pTempClient) {
        pTempClient->disconnect();
        delete pTempClient;
    }
    pTempClient = new BtClient(this);
    pTempClient->startClient(serviceInfo);
    connect(pTempClient, SIGNAL(connected(QString)),
            this, SLOT(onPanelClientConnected(QString)));
}


void
BtScoreController::discoveryFinished() {
    if(!pPanelClient)
        try2ConnectBt();
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

    sToken = XML_Parse(sMessage, "startSpotLoop");
    if(sToken != sNoData) {
        QPixmap pixmap(":/ButtonIcons/sign_stop.png");
        QIcon ButtonIcon(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setDisabled(true);
        pGeneralSetupButton->setDisabled(true);
        myStatus = showSpots;
    }

    sToken = XML_Parse(sMessage, "endSpotLoop");
    if(sToken != sNoData) {
        QPixmap pixmap(":/ButtonIcons/PlaySpots.png");
        QIcon ButtonIcon(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
        myStatus = showPanel;
    }

    sToken = XML_Parse(sMessage, "startSlideShow");
    if(sToken != sNoData) {
        QPixmap pixmap(":/ButtonIcons/sign_stop.png");
        QIcon ButtonIcon(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        pSpotButton->setDisabled(true);
        pGeneralSetupButton->setDisabled(true);
        myStatus = showSpots;
    }

    sToken = XML_Parse(sMessage, "endSlideShow");
    if(sToken != sNoData) {
        QPixmap pixmap(":/ButtonIcons/PlaySlides.png");
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
    Q_UNUSED(sName)
    stopBtDiscovery();
    pPanelClient = pTempClient;
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
    setDisabled(true);
    if(pPanelClient) {
        pPanelClient->disconnect();
        pPanelClient->deleteLater();
        pPanelClient = nullptr;
    }
    try2ConnectBt();
}


void
BtScoreController::onPanelClientSocketError(QString sError) {
    Q_UNUSED(sError)
    setDisabled(true);
    if(pPanelClient) {
        pPanelClient->disconnect();
        pPanelClient->deleteLater();
        pPanelClient = nullptr;
    }
    try2ConnectBt();
}
