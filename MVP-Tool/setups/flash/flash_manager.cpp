#include "flash_manager.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

FlashManager::FlashManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(nullptr)
    , m_isConnected(false)
    , m_isFlashing(false)
    , m_flashProgress(0)
{
    m_serialPort = new QSerialPort(this);
    m_flashTimer = new QTimer(this);
    
    // 连接信号槽
    connect(m_serialPort, &QSerialPort::readyRead, this, &FlashManager::onSerialDataReceived);
    connect(m_flashTimer, &QTimer::timeout, this, &FlashManager::onFlashTimeout);
    
    m_flashTimer->setSingleShot(true);
}

FlashManager::~FlashManager()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool FlashManager::selectFirmwareFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        setError("固件文件不存在或无效");
        return false;
    }
    
    if (fileInfo.suffix().toLower() != "bin") {
        setError("固件文件必须是.bin格式");
        return false;
    }
    
    m_firmwarePath = filePath;
    emit firmwareSelected(fileInfo.fileName());
    return true;
}

bool FlashManager::selectSecureBootFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        setError("安全启动文件不存在或无效");
        return false;
    }
    
    if (fileInfo.suffix().toLower() != "zip") {
        setError("安全启动文件必须是.zip格式");
        return false;
    }
    
    m_secureBootPath = filePath;
    emit secureBootSelected(fileInfo.fileName());
    return true;
}

bool FlashManager::selectConfigFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        setError("配置文件不存在或无效");
        return false;
    }
    
    m_configPath = filePath;
    return true;
}

bool FlashManager::openSerialPort(const QString &portName, int baudRate)
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_isConnected = true;
        emit serialConnected(true);
        emit flashStatus("串口连接成功");
        return true;
    } else {
        setError("无法打开串口: " + m_serialPort->errorString());
        m_isConnected = false;
        emit serialConnected(false);
        return false;
    }
}

void FlashManager::closeSerialPort()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
        m_isConnected = false;
        emit serialConnected(false);
        emit flashStatus("串口已断开");
    }
}

bool FlashManager::startFlash()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isConnected) {
        setError("串口未连接");
        return false;
    }
    
    if (m_firmwarePath.isEmpty()) {
        setError("请先选择固件文件");
        return false;
    }
    
    if (m_isFlashing) {
        setError("烧录已在进行中");
        return false;
    }
    
    m_isFlashing = true;
    m_flashProgress = 0;
    emit flashStatus("开始烧录...");
    emit flashProgress(0);
    
    // 启动烧录定时器
    m_flashTimer->start(100); // 100ms更新一次进度
    
    return true;
}

void FlashManager::stopFlash()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isFlashing) {
        m_isFlashing = false;
        m_flashTimer->stop();
        emit flashStatus("烧录已停止");
        emit flashFinished(false, "用户停止烧录");
    }
}

bool FlashManager::isConnected() const
{
    return m_isConnected;
}

bool FlashManager::isFlashing() const
{
    return m_isFlashing;
}

QString FlashManager::getLastError() const
{
    return m_lastError;
}

void FlashManager::onSerialDataReceived()
{
    if (!m_isFlashing) return;
    
    QByteArray data = m_serialPort->readAll();
    qDebug() << "收到串口数据:" << data.toHex();
    
    // 这里可以添加具体的烧录协议解析逻辑
    // 根据MVP3的烧录协议进行响应处理
}

void FlashManager::onFlashTimeout()
{
    if (!m_isFlashing) return;
    
    // 模拟烧录进度更新
    m_flashProgress += 5;
    if (m_flashProgress >= 100) {
        m_flashProgress = 100;
        m_isFlashing = false;
        emit flashProgress(100);
        emit flashStatus("烧录完成");
        emit flashFinished(true, "烧录成功完成");
    } else {
        emit flashProgress(m_flashProgress);
        emit flashStatus(QString("烧录中... %1%").arg(m_flashProgress));
        m_flashTimer->start(100);
    }
}

bool FlashManager::sendFlashCommand(const QByteArray &command)
{
    if (!m_isConnected || !m_serialPort->isOpen()) {
        return false;
    }
    
    qint64 bytesWritten = m_serialPort->write(command);
    return bytesWritten == command.size();
}

bool FlashManager::verifyFlashResponse(const QByteArray &expected)
{
    // 这里实现响应验证逻辑
    return true;
}

void FlashManager::updateProgress(int progress)
{
    m_flashProgress = progress;
    emit flashProgress(progress);
}

void FlashManager::setError(const QString &error)
{
    m_lastError = error;
    qDebug() << "FlashManager Error:" << error;
}
