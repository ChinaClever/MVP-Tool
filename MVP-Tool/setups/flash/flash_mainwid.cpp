#include "flash_mainwid.h"
#include "ui_flash_mainwid.h"
#include <QSerialPortInfo>
#include <QApplication>
#include <QStyle>
#include <QDateTime>
#include <QFileInfo>

Flash_MainWid::Flash_MainWid(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Flash_MainWid)
    , m_flashManager(new FlashManager(this))
    , m_isConnected(false)
    , m_isFlashing(false)
{
    ui->setupUi(this);
    initUI();
    setupConnections();
    refreshSerialPorts();
    updateUI();
}

Flash_MainWid::~Flash_MainWid()
{
    delete ui;
}

void Flash_MainWid::initUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 串口连接组
    QGroupBox *connectionGroup = new QGroupBox("串口连接", this);
    QGridLayout *connectionLayout = new QGridLayout(connectionGroup);
    
    connectionLayout->addWidget(new QLabel("串口:"), 0, 0);
    m_serialPortCombo = new QComboBox(this);
    connectionLayout->addWidget(m_serialPortCombo, 0, 1);
    
    connectionLayout->addWidget(new QLabel("波特率:"), 0, 2);
    m_baudRateCombo = new QComboBox(this);
    m_baudRateCombo->addItems({"9600", "19200", "38400", "57600", "115200"});
    m_baudRateCombo->setCurrentText("115200");
    connectionLayout->addWidget(m_baudRateCombo, 0, 3);
    
    m_connectBtn = new QPushButton("连接", this);
    m_disconnectBtn = new QPushButton("断开", this);
    connectionLayout->addWidget(m_connectBtn, 0, 4);
    connectionLayout->addWidget(m_disconnectBtn, 0, 5);
    
    mainLayout->addWidget(connectionGroup);
    
    // 文件选择组
    QGroupBox *fileGroup = new QGroupBox("烧录文件", this);
    QGridLayout *fileLayout = new QGridLayout(fileGroup);
    
    fileLayout->addWidget(new QLabel("固件文件:"), 0, 0);
    m_firmwarePathEdit = new QLineEdit(this);
    m_firmwarePathEdit->setPlaceholderText("选择固件文件 (.bin)");
    fileLayout->addWidget(m_firmwarePathEdit, 0, 1, 1, 3);
    m_selectFirmwareBtn = new QPushButton("浏览", this);
    fileLayout->addWidget(m_selectFirmwareBtn, 0, 4);
    
    fileLayout->addWidget(new QLabel("安全启动:"), 1, 0);
    m_selectSecureBootBtn = new QPushButton("选择安全启动包", this);
    fileLayout->addWidget(m_selectSecureBootBtn, 1, 1, 1, 4);
    
    fileLayout->addWidget(new QLabel("配置文件:"), 2, 0);
    m_selectConfigBtn = new QPushButton("选择配置文件", this);
    fileLayout->addWidget(m_selectConfigBtn, 2, 1, 1, 4);
    
    mainLayout->addWidget(fileGroup);
    
    // 烧录控制组
    QGroupBox *controlGroup = new QGroupBox("烧录控制", this);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
    
    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLayout->addWidget(new QLabel("进度:"));
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    progressLayout->addWidget(m_progressBar);
    controlLayout->addLayout(progressLayout);
    
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(new QLabel("状态:"));
    m_statusLabel = new QLabel("未连接", this);
    statusLayout->addWidget(m_statusLabel);
    controlLayout->addLayout(statusLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_startBtn = new QPushButton("开始烧录", this);
    m_stopBtn = new QPushButton("停止烧录", this);
    buttonLayout->addWidget(m_startBtn);
    buttonLayout->addWidget(m_stopBtn);
    buttonLayout->addStretch();
    controlLayout->addLayout(buttonLayout);
    
    mainLayout->addWidget(controlGroup);
    
    // 日志组
    QGroupBox *logGroup = new QGroupBox("操作日志", this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    
    m_logText = new QTextEdit(this);
    m_logText->setMaximumHeight(150);
    m_logText->setReadOnly(true);
    logLayout->addWidget(m_logText);
    
    mainLayout->addWidget(logGroup);
    
    // 设置窗口标题
    setWindowTitle("MVP3 烧录工具");
    setMinimumSize(600, 500);
}

void Flash_MainWid::setupConnections()
{
    // 按钮信号槽
    connect(m_connectBtn, &QPushButton::clicked, this, &Flash_MainWid::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &Flash_MainWid::onConnectClicked);
    connect(m_selectFirmwareBtn, &QPushButton::clicked, this, &Flash_MainWid::onSelectFirmwareClicked);
    connect(m_selectSecureBootBtn, &QPushButton::clicked, this, &Flash_MainWid::onSelectSecureBootClicked);
    connect(m_selectConfigBtn, &QPushButton::clicked, this, &Flash_MainWid::onSelectConfigClicked);
    connect(m_startBtn, &QPushButton::clicked, this, &Flash_MainWid::onStartFlashClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &Flash_MainWid::onStopFlashClicked);
    
    // FlashManager信号槽
    connect(m_flashManager, &FlashManager::flashProgress, this, &Flash_MainWid::onFlashProgress);
    connect(m_flashManager, &FlashManager::flashStatus, this, &Flash_MainWid::onFlashStatus);
    connect(m_flashManager, &FlashManager::flashFinished, this, &Flash_MainWid::onFlashFinished);
    connect(m_flashManager, &FlashManager::serialConnected, this, &Flash_MainWid::onSerialConnected);
    connect(m_flashManager, &FlashManager::firmwareSelected, this, &Flash_MainWid::onFirmwareSelected);
    connect(m_flashManager, &FlashManager::secureBootSelected, this, &Flash_MainWid::onSecureBootSelected);
}

void Flash_MainWid::updateUI()
{
    m_connectBtn->setEnabled(!m_isConnected);
    m_disconnectBtn->setEnabled(m_isConnected);
    m_startBtn->setEnabled(m_isConnected && !m_isFlashing);
    m_stopBtn->setEnabled(m_isFlashing);
    
    m_serialPortCombo->setEnabled(!m_isConnected);
    m_baudRateCombo->setEnabled(!m_isConnected);
}

void Flash_MainWid::refreshSerialPorts()
{
    m_serialPortCombo->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        m_serialPortCombo->addItem(info.portName() + " - " + info.description());
    }
}

void Flash_MainWid::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logText->append(QString("[%1] %2").arg(timestamp, message));
}

void Flash_MainWid::onSelectFirmwareClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择固件文件", "", "固件文件 (*.bin)");
    if (!filePath.isEmpty()) {
        if (m_flashManager->selectFirmwareFile(filePath)) {
            m_firmwarePathEdit->setText(filePath);
            logMessage("固件文件已选择: " + QFileInfo(filePath).fileName());
        } else {
            QMessageBox::warning(this, "错误", m_flashManager->getLastError());
        }
    }
}

void Flash_MainWid::onSelectSecureBootClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择安全启动包", "", "安全启动包 (*.zip)");
    if (!filePath.isEmpty()) {
        if (m_flashManager->selectSecureBootFile(filePath)) {
            logMessage("安全启动包已选择: " + QFileInfo(filePath).fileName());
        } else {
            QMessageBox::warning(this, "错误", m_flashManager->getLastError());
        }
    }
}

void Flash_MainWid::onSelectConfigClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择配置文件", "", "配置文件 (*.cfg *.xml)");
    if (!filePath.isEmpty()) {
        if (m_flashManager->selectConfigFile(filePath)) {
            logMessage("配置文件已选择: " + QFileInfo(filePath).fileName());
        } else {
            QMessageBox::warning(this, "错误", m_flashManager->getLastError());
        }
    }
}

void Flash_MainWid::onConnectClicked()
{
    if (!m_isConnected) {
        // 连接串口
        QString portName = m_serialPortCombo->currentText().split(" - ").first();
        int baudRate = m_baudRateCombo->currentText().toInt();
        
        if (m_flashManager->openSerialPort(portName, baudRate)) {
            m_isConnected = true;
            logMessage("串口连接成功: " + portName);
        } else {
            QMessageBox::warning(this, "连接失败", m_flashManager->getLastError());
        }
    } else {
        // 断开串口
        m_flashManager->closeSerialPort();
        m_isConnected = false;
        logMessage("串口已断开");
    }
    
    updateUI();
}

void Flash_MainWid::onStartFlashClicked()
{
    if (m_flashManager->startFlash()) {
        m_isFlashing = true;
        logMessage("开始烧录...");
    } else {
        QMessageBox::warning(this, "烧录失败", m_flashManager->getLastError());
    }
    
    updateUI();
}

void Flash_MainWid::onStopFlashClicked()
{
    m_flashManager->stopFlash();
    m_isFlashing = false;
    logMessage("烧录已停止");
    updateUI();
}

void Flash_MainWid::onFlashProgress(int percentage)
{
    m_progressBar->setValue(percentage);
}

void Flash_MainWid::onFlashStatus(const QString &status)
{
    m_statusLabel->setText(status);
    logMessage(status);
}

void Flash_MainWid::onFlashFinished(bool success, const QString &message)
{
    m_isFlashing = false;
    m_statusLabel->setText(success ? "烧录完成" : "烧录失败");
    logMessage(message);
    
    if (success) {
        QMessageBox::information(this, "烧录完成", "固件烧录成功完成！");
    } else {
        QMessageBox::warning(this, "烧录失败", message);
    }
    
    updateUI();
}

void Flash_MainWid::onSerialConnected(bool connected)
{
    m_isConnected = connected;
    updateUI();
}

void Flash_MainWid::onFirmwareSelected(const QString &fileName)
{
    logMessage("固件文件已选择: " + fileName);
}

void Flash_MainWid::onSecureBootSelected(const QString &fileName)
{
    logMessage("安全启动包已选择: " + fileName);
}
