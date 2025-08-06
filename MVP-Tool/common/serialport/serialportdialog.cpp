#include "serialportdialog.h"
#include "ui_serialportdialog.h"

#include <QSerialPortInfo>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

SerialPortDialog::SerialPortDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialPortDialog)
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowTitle("串口选择");
    setFixedSize(400, 200);

    // 填充波特率列表
    populateBaudRates();

    // 初始刷新串口
    refreshCOMPorts();

    // 连接信号槽
    connect(ui->refreshButton, &QPushButton::clicked, this, &SerialPortDialog::refreshCOMPorts);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SerialPortDialog::acceptSelection);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

SerialPortDialog::~SerialPortDialog()
{
    delete ui;
}

QList<QSerialPortInfo> SerialPortDialog::getCOMPorts() const
{
    QList<QSerialPortInfo> comPorts;

    // 获取所有串口并过滤只保留COM开头的
    foreach (const QSerialPortInfo &port, QSerialPortInfo::availablePorts()) {
        if (port.portName().startsWith("COM")) {
            comPorts.append(port);
        }
    }

    return comPorts;
}

void SerialPortDialog::populateBaudRates()
{
    // 常用波特率
    QList<int> baudRates = {
        1200, 2400, 4800, 9600,
        19200, 38400, 57600,
        115200, 230400, 460800, 921600
    };

    foreach (int rate, baudRates) {
        ui->baudComboBox->addItem(QString::number(rate), rate);
    }

    // 默认选择115200
    ui->baudComboBox->setCurrentIndex(baudRates.indexOf(115200));
}

void SerialPortDialog::refreshCOMPorts()
{
    // 保存当前选择的串口
    QString currentSelection = ui->portComboBox->currentText();

    // 清空并重新填充
    ui->portComboBox->clear();

    QList<QSerialPortInfo> comPorts = getCOMPorts();

    if (comPorts.isEmpty()) {
        ui->portComboBox->addItem("未检测到COM串口");
        ui->portComboBox->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        foreach (const QSerialPortInfo &port, comPorts) {
            QString displayText = QString("%1: %2")
            .arg(port.portName())
                .arg(port.description());
            ui->portComboBox->addItem(displayText, port.portName());
        }
        ui->portComboBox->setEnabled(true);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

        // 恢复之前的选中项（如果存在）
        if (!currentSelection.isEmpty()) {
            int index = ui->portComboBox->findData(currentSelection);
            if (index >= 0) {
                ui->portComboBox->setCurrentIndex(index);
            }
        }
    }
}

void SerialPortDialog::acceptSelection()
{
    if (ui->portComboBox->isEnabled()) {
        m_selectedPort = ui->portComboBox->currentData().toString();
        m_selectedBaudRate = ui->baudComboBox->currentData().toInt();
        accept();
    } else {
        QMessageBox::warning(this, "警告", "没有可用的COM串口");
    }
}

QString SerialPortDialog::selectedPort() const
{
    return m_selectedPort;
}

int SerialPortDialog::selectedBaudRate() const
{
    return m_selectedBaudRate;
}
