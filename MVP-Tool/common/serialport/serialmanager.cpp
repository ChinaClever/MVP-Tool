#include "SerialManager.h"
#include "qserialport.h"
#include <QDebug>

SerialManager& SerialManager::instance()
{
    static SerialManager mgr;
    return mgr;
}

SerialManager::SerialManager(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);
}

SerialManager::~SerialManager()
{
    if(serial->isOpen()) serial->close();
}

bool SerialManager::isOpen() const {
    return serial && serial->isOpen();
}

bool SerialManager::openSerial(const QString &portName, int baudRate)
{
    if(serial->isOpen()) serial->close();

    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if(serial->open(QIODevice::ReadWrite)) {
        qDebug() << "串口已成功打开:" << portName;
        emit serialOpened(portName);
        return true;
    } else {
        qWarning() << "串口打开失败:" << serial->errorString();
        emit serialOpenFailed(serial->errorString());
        return false;
    }
}

bool SerialManager::writeData(const QByteArray &data = "cat /etodisk/system.cfg\r\n")
{
    if(serial && serial->isOpen())
        return serial->write(data) == data.size();
    return false;
}

QByteArray SerialManager::readAll()
{
    if(serial && serial->isOpen())
        return serial->readAll();
    return QByteArray();
}
