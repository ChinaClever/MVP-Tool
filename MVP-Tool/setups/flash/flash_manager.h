#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QThread>
#include <QMutex>
#include <QTimer>

class FlashManager : public QObject
{
    Q_OBJECT

public:
    explicit FlashManager(QObject *parent = nullptr);
    ~FlashManager();

    // 烧录相关方法
    bool selectFirmwareFile(const QString &filePath);
    bool selectSecureBootFile(const QString &filePath);
    bool selectConfigFile(const QString &filePath);
    
    // 串口通信
    bool openSerialPort(const QString &portName, int baudRate = 115200);
    void closeSerialPort();
    
    // 烧录操作
    bool startFlash();
    void stopFlash();
    
    // 状态查询
    bool isConnected() const;
    bool isFlashing() const;
    QString getLastError() const;

signals:
    void flashProgress(int percentage);
    void flashStatus(const QString &status);
    void flashFinished(bool success, const QString &message);
    void serialConnected(bool connected);
    void firmwareSelected(const QString &fileName);
    void secureBootSelected(const QString &fileName);

private slots:
    void onSerialDataReceived();
    void onFlashTimeout();

private:
    // 烧录文件
    QString m_firmwarePath;
    QString m_secureBootPath;
    QString m_configPath;
    
    // 串口通信
    QSerialPort *m_serialPort;
    bool m_isConnected;
    
    // 烧录状态
    bool m_isFlashing;
    int m_flashProgress;
    QString m_lastError;
    
    // 线程安全
    QMutex m_mutex;
    QTimer *m_flashTimer;
    
    // 私有方法
    bool sendFlashCommand(const QByteArray &command);
    bool verifyFlashResponse(const QByteArray &expected);
    void updateProgress(int progress);
    void setError(const QString &error);
};

#endif // FLASH_MANAGER_H
