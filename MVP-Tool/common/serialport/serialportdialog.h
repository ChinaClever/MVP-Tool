#ifndef SERIALPORTDIALOG_H
#define SERIALPORTDIALOG_H

#include <QDialog>
#include <QSerialPortInfo>

namespace Ui {
class SerialPortDialog;
}

class SerialPortDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SerialPortDialog(QWidget *parent = nullptr);
    ~SerialPortDialog();

    // 获取用户选择的串口
    QString selectedPort() const;

    // 获取用户选择的波特率
    int selectedBaudRate() const;

private slots:
    // 刷新COM串口列表
    void refreshCOMPorts();

    // 验证并接受选择
    void acceptSelection();

private:
    Ui::SerialPortDialog *ui;
    QString m_selectedPort;
    int m_selectedBaudRate;

    // 填充波特率下拉框
    void populateBaudRates();

    // 获取所有COM开头的串口
    QList<QSerialPortInfo> getCOMPorts() const;
};

#endif // SERIALPORTDIALOG_H
