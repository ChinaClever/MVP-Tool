#include "serialstatuswid.h"
#include "ui_serialstatuswid.h"
#include "serialportdialog.h"

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

        qDebug() << "Selected port:" << portName;
        qDebug() << "Selected baud rate:" << baudRate;

        // 调用串口连接函数
        setupSerialConnection(portName, baudRate);
    }
}

void SerialStatusWid::setupSerialConnection(const QString &portName, int baudRate)
{
    // 如果已有串口对象，先关闭并删除
    if(serial) {
        if(serial->isOpen()) {
            serial->close();
        }
        delete serial;   // 删除对象
        serial = nullptr;
    }

    // 创建新的串口对象，注意指定父对象为this，以便在SerialStatusWid销毁时自动删除
    serial = new QSerialPort(this);  // 重新创建对象

    serial->setPortName(portName);
    serial->setBaudRate(baudRate);

    // 设置其他参数
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);


    qDebug() << "串口已成功打开:" << portName;

    ui->serialLab->setText(portName+"串口已打开");
    comPort = portName;

}
