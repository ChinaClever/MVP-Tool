#ifndef FLASH_MAINWID_H
#define FLASH_MAINWID_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include "flash_manager.h"

namespace Ui {
class Flash_MainWid;
}

class Flash_MainWid : public QWidget
{
    Q_OBJECT

public:
    explicit Flash_MainWid(QWidget *parent = nullptr);
    ~Flash_MainWid();

private slots:
    void onSelectFirmwareClicked();
    void onSelectSecureBootClicked();
    void onSelectConfigClicked();
    void onConnectClicked();
    void onStartFlashClicked();
    void onStopFlashClicked();
    
    // FlashManager信号槽
    void onFlashProgress(int percentage);
    void onFlashStatus(const QString &status);
    void onFlashFinished(bool success, const QString &message);
    void onSerialConnected(bool connected);
    void onFirmwareSelected(const QString &fileName);
    void onSecureBootSelected(const QString &fileName);

private:
    void initUI();
    void setupConnections();
    void updateUI();
    void refreshSerialPorts();
    void logMessage(const QString &message);

private:
    Ui::Flash_MainWid *ui;
    FlashManager *m_flashManager;
    
    // UI控件
    QComboBox *m_serialPortCombo;
    QComboBox *m_baudRateCombo;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    
    QLineEdit *m_firmwarePathEdit;
    QPushButton *m_selectFirmwareBtn;
    QPushButton *m_selectSecureBootBtn;
    QPushButton *m_selectConfigBtn;
    
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    
    QTextEdit *m_logText;
    
    // 状态
    bool m_isConnected;
    bool m_isFlashing;
};

#endif // FLASH_MAINWID_H
