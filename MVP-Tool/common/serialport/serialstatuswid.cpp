#include "serialstatuswid.h"
#include "ui_serialstatuswid.h"
#include "serialportdialog.h"
#include "serialmanager.h"
#include <QDebug>
extern QString comPort;

SerialStatusWid::SerialStatusWid(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SerialStatusWid)
    , serial (new QSerialPort(this))
{
    ui->setupUi(this);
}

SerialStatusWid::~SerialStatusWid()
{
    delete ui;
}

void SerialStatusWid::on_comBtn_clicked()
{
    SerialPortDialog dialog(this);
    if(dialog.exec() == QDialog::Accepted) {
        QString portName = dialog.selectedPort();
        int baudRate = dialog.selectedBaudRate();

        bool ok = SerialManager::instance().openSerial(portName, baudRate);
        if(ok) ui->serialLab->setText(portName + " 串口已打开");
        else ui->serialLab->setText("串口打开失败");
    }
}
