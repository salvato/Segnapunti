#include <QLabel>
#include <QGridLayout>

#include "remainingtimedialog.h"


RemainingTimeDialog::RemainingTimeDialog() {
    sCorrectFormat = QString("background:white;color:black;");
    sErrorFormat   = QString("background:red;color:white;");

    QLabel* pMinutesLabel = new QLabel(tr("Minuti:"));
    QLabel* pSecondsLabel = new QLabel(tr("Secondi:"));

    minuteEdit.setMaxLength(1);
    minuteEdit.setInputMask("9");  // Only numbers
    secondsEdit.setMaxLength(2);
    secondsEdit.setInputMask("99");// Only Numbers

    buttonOk.setText("Ok");
    buttonCancel.setText("Cancel");

    connect(&minuteEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onMinuteChanged(QString)));
    connect(&secondsEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onSecondsChanged(QString)));

    connect(&buttonOk, SIGNAL(clicked()),
            this, SLOT(onOkClicked()));
    connect(&buttonCancel, SIGNAL(clicked()),
            this, SLOT(onCancelClicked()));

    QGridLayout* pMainLayout = new QGridLayout;
    pMainLayout->addWidget(pMinutesLabel, 0, 0, 1, 1);
    pMainLayout->addWidget(&minuteEdit,   0, 1, 1, 1);

    pMainLayout->addWidget(pSecondsLabel, 1, 0, 1, 1);
    pMainLayout->addWidget(&secondsEdit,  1, 1, 1, 1);

    pMainLayout->addWidget(&buttonCancel, 2, 0, 1, 1);
    pMainLayout->addWidget(&buttonOk,     2, 1, 1, 1);

    setLayout(pMainLayout);
}


void
RemainingTimeDialog::onCancelClicked() {
    QDialog::reject();
}


void
RemainingTimeDialog::onOkClicked() {
    QDialog::accept();
}


void
RemainingTimeDialog::onMinuteChanged(QString sText) {
    if((sText.toInt() >= 0) &&
       (sText.toInt() < 9))
    {
        minuteEdit.setStyleSheet(sCorrectFormat);
        buttonOk.setEnabled(true);
    }
    else {
        minuteEdit.setStyleSheet(sErrorFormat);
        buttonOk.setEnabled(false);
    }
}


void
RemainingTimeDialog::onSecondsChanged(QString sText) {
    if((sText.toInt() >= 0) &&
       (sText.toInt() < 60))
    {
        secondsEdit.setStyleSheet(sCorrectFormat);
        buttonOk.setEnabled(true);
    }
    else {
        secondsEdit.setStyleSheet(sErrorFormat);
        buttonOk.setEnabled(false);
    }
}


void
RemainingTimeDialog::setMinutes(int minutes) {
    minuteEdit.setText(QString("%1").arg(minutes, 1));
}


void
RemainingTimeDialog::setSeconds(int seconds) {
    secondsEdit.setText(QString("%1").arg(seconds, 1));
}


int
RemainingTimeDialog::getMinutes() {
    return minuteEdit.text().toInt();
}


int
RemainingTimeDialog::getSeconds() {
    return secondsEdit.text().toInt();
}
