#pragma once
#include <QSerialPort>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialManager : public QObject
{
    Q_OBJECT
public:
    static SerialManager& instance();

    bool openSerial(const QString &portName, int baudRate);
    bool writeData(const QByteArray &data);
    QByteArray readAll();
    QSerialPort* serialPort() { return serial; }
    bool isOpen() const;

signals:
    void serialOpened(const QString &portName);
    void serialOpenFailed(const QString &err);

private:
    SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    QSerialPort *serial;
};
