#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QScrollBar>
#include <QTimer>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , process(nullptr)
{
    ui->setupUi(this);

    QStringList testItems = {
        "全部测试", "蜂鸣器测试 ", "实时时钟测试", "调试LED测试",
        "PCB温度传感器", "安全元件测试",
        "蓝牙测试", "以太网测试", " SPE接口测试", "Zigbee测试",
        "RGB灯测试", " RS485/Modbus测试", " 传感器测试", "USB接口测试"
    };

    // 新增参数映射表
    m_testArgMap = {
        {"全部测试", "0"},
        {"蜂鸣器测试", "1"},
        {"实时时钟测试", "2"},
        {"调试LED测试", "3"},
        {"PCB温度传感器", "4"},
        {"安全元件测试", "5"},
        {"蓝牙测试", "6"},
        {"以太网测试", "7"},
        {"SPE接口测试", "8"},
        {"Zigbee测试", "9"},
        {"RGB灯测试", "10"},
        {"RS485/Modbus测试", "11"},
        {"传感器测试", "12"},
        {"USB接口测试", "13"}
    };
    // 设置每个项的文本对齐属性

    ui->TestItems->addItems(testItems);
    connect(ui->startTestButton, &QPushButton::clicked, this, &MainWindow::onStartTestClicked);
    connect(ui->TestItems, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTestItemChanged);
    connect(ui->clearLogBtn, &QPushButton::clicked, this, &MainWindow::onclearLogBtnclicked);


}

MainWindow::~MainWindow()
{
    delete process;
    process =nullptr;
    delete ui;
}

void MainWindow::onStartTestClicked()
{
    onclearLogBtnclicked();
    //获取端口
    QString comPort = ui->comPortEdit->text().trimmed();
    // 获取当前选中的测试项
    QString selectedTest = ui->TestItems->currentText().trimmed(); // 去除前后空格
    QString arg = m_testArgMap.value(selectedTest, "0"); // 获取参数


    if(!validateComPort(comPort)) {
        return;
    }

    //检查脚本是否存在
    QString pythonInterpreter = "python3";
    QString scriptPath = "D:/test/word_test/MVP-Tool/MVP3.py";
  //  QString scriptPath = "‪D:/test/word_test/MVP-Tool/MVP_3.py";
    scriptPath = scriptPath.trimmed(); // 移除首尾空白
    scriptPath.replace(QRegularExpression("[^\\x20-\\x7E]"), ""); // 过滤非ASCII字符
    if (!QFile::exists(scriptPath)) {
        QMessageBox::critical(this, "错误", "找不到测试脚本: MVP3.py");
        return;
    }
    // 初始化日志和UI状态
    ui->logText->appendPlainText("══════════════════════════");
    ui->logText->appendPlainText(" 开始测试: " + selectedTest);
    ui->logText->appendPlainText(" COM端口: " + comPort);
    ui->startTestButton->setEnabled(false);
    ui->startTestButton->setText("测试中...");
    ui->logText->appendPlainText("  正在测试,请勿关闭   \n");

    //创建一个新进程
    process = new QProcess(this);
    // 设置输出通道模式：合并标准输出和错误输出
    process->setProcessChannelMode(QProcess::MergedChannels);


    connect(process, &QProcess::readyReadStandardOutput, [=]() {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        output.replace(QRegularExpression("\x1B\\[[0-?]*[ -/]*[@-~]"), ""); // 过滤ANSI
        if (!output.trimmed().isEmpty()) {
            ui->logText->appendPlainText("[输出] " + output.trimmed());
            // 自动滚动到底部
            QScrollBar* sb = ui->logText->verticalScrollBar();
            sb->setValue(sb->maximum());
        }
    });

    // 进程结束处理
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int code, QProcess::ExitStatus status) {
                QString result = (code == 0)
                ? "✅ 测试成功"
                : "❌ 测试失败 (错误码: " + QString::number(code) + ")";
                ui->logText->appendPlainText(result);
                ui->startTestButton->setEnabled(true);
                ui->startTestButton->setText("开始测试");
                process->deleteLater();
                process = nullptr;
            });

    // 启动进程
    QStringList args;
    args << "-u"
         << scriptPath   // 脚本路径
         << comPort      // 第一个参数：COM端口号
         << arg;     // 第二个参数：测试项编号



    process->start("python", args);

    // 进程启动失败处理
    if (!process->waitForStarted(2000)) {
        ui->logText->appendPlainText("⚠️ 无法启动进程：");
        ui->logText->appendPlainText(process->errorString());
        ui->startTestButton->setEnabled(true);
        process->deleteLater();
    }


}

void MainWindow::onTestItemChanged(int index)
{

}

void MainWindow::onclearLogBtnclicked()
{
    ui->logText->clear();  // 清空 QPlainTextEdit 的内容
}

//验证用户输入的COM端口是否合法

bool MainWindow::validateComPort(const QString& comPort)
{
    if(comPort.isEmpty()) {
        QMessageBox::critical(this, "错误", "COM端口号不能为空");
        return false;
    }

    bool ok;
    int port = comPort.toInt(&ok);
    if(!ok) {
        QMessageBox::critical(this, "错误", "请输入有效的数字端口号");
        return false;
    }

    if(port < 1 || port > 99) {
        QMessageBox::critical(this, "错误", "端口号范围应为1-99");
        return false;
    }

    return true;
}


void MainWindow::on_Yes_Btn_clicked()
{
    if (process && process->state() == QProcess::Running) {
        qDebug() << "YES";
        process->write("Y\n");
    } else {
        qDebug() << "⚠️ 测试进程未启动，忽略 Y 按钮操作";
        // 可选：提示用户或日志记录
        // QMessageBox::information(this, "提示", "测试尚未开始，无法发送 Y 指令。");
    }
}



void MainWindow::on_No_Btn_clicked()
{
    if (process && process->state() == QProcess::Running) {
        qDebug() << "NO";
        process->write("N\n");
    } else {
        qDebug() << "⚠️ 测试进程未启动，忽略 N 按钮操作";
        // 可选：提示用户或日志记录
        // QMessageBox::information(this, "提示", "测试尚未开始，无法发送 N 指令。");
    }
}


