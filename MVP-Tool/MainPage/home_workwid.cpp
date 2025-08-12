#include "home_workwid.h"
#include "ui_home_workwid.h"
#include "createini.h"
#include "common/globals/globals.h"
#include "baselogs.h"
#include "backcolour/backcolourcom.h"
#include "msgbox.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QScrollBar>
#include <QMessageBox>
Home_WorkWid::Home_WorkWid(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Home_WorkWid)
    , process(nullptr)
{
    ui->setupUi(this);
    set_background_icon(this,":/image/box_back.jpg");
        timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&Home_WorkWid::updateTime);
    initFunSlot();
    mCoreThread = new Test_CoreThread(this);
    connect(mCoreThread,&Test_CoreThread::updateLcd,this,&Home_WorkWid::updateLcd);
}

Home_WorkWid::~Home_WorkWid()
{
    if (process) {
        process->kill();
        if (!process->waitForFinished(3000)) {
            process->terminate();
            process->waitForFinished();
        }
        delete process;
        process = nullptr;
    }

    delete ui; // 最后删除 UI
}

bool Home_WorkWid::intiarg()
{
    pythonInterpreter = "python";
    QString comport = comPort.trimmed();
     digits = comport.right(comport.length() - 3);
    if(!validateComPort(digits)) {
        return false;
    }

    arg = QString::number(ui->modeBox->currentIndex());
    scriptPath = "D:/test/word_test/MVP3/MVP-Tool/MVP3.py";

    if (!QFile::exists(scriptPath)) {
        QMessageBox::critical(this, "错误", "找不到测试脚本: MVP3.exe");
        return false;
    }

    return true;
}

void Home_WorkWid::uiClear()
{
    // 清空所有 QLabel 控件的文本
    ui->eth1Lab->setText("--- ---");
    ui->hwLab->setText("--- ---");
    ui->spe1Lab->setText("--- ---");
    ui->macLab->setText("--- ---");
    ui->btLab->setText("--- ---");
    ui->fwLab->setText("--- ---");
    ui->snLab->setText("--- ---");
    ui->spe0Lab->setText("--- ---");
    ui->zbLab->setText("--- ---");
    ui->eth0Lab->setText("--- ---");
    ui->eth2Lab->setText("--- ---");
    ui->textEdit->clear();
}

bool Home_WorkWid::pcbCheck()
{
    //qDebug()<<ui->pcbCode->text().size();
    if(ui->pcbCode->text().isEmpty()){
        MsgBox::critical(0, tr("请输入pcb码"));
        return 0;
    }
    mDev->dt.pcbCode = ui->pcbCode->text();
    return 1;
}

void Home_WorkWid::on_startBtn_clicked()
{

    if(ui->startBtn->text() == "开始测试"){

        mPacket->init();
        if(!pcbCheck())return ;
        allTestState = true;
        if(!intiarg()) return; //寻找 MVP3 py脚本
        if(arg == 0) mPro->allTest = 1;
        uiClear();
        ui->startBtn->setText("测试中...");
        updateTime();
        mDev->dt.date = QDateTime::currentDateTime().toString("HH:mm:ss");

        if(arg == "0"){
            timer->setInterval(1000);
            timer->start();
        }

        intiTest();

        workProcess();
    }
    else{
        QMessageBox::StandardButton ret = QMessageBox::question(this, tr("确认"), tr("确定需要提前结束？"),
                                         QMessageBox::Yes | QMessageBox::No);

        if(ret == QMessageBox::Yes) {
            ui->startBtn->setText("开始");
            mPro->result = Test_Fail;
            qDebug()<<mPro->result;
            process->kill();
            ui->textEdit->append("提前结束");
            mDev->dt.reason += "  --手动提前结束";
            //updateResult();
        }
    }

}

void Home_WorkWid::initFunSlot()
{
    mPro->step = Test_End;
}

void Home_WorkWid::workProcess()
{
    if (process) {
        disconnect(process, nullptr, this, nullptr); // 断开所有与 process 相关的连接
        process->kill();
        process->deleteLater();
        process = nullptr;
    }

    process = new QProcess(this);

    // 标准输出处理（连接handle_stdout）
    connect(process, &QProcess::readyReadStandardOutput, this, &Home_WorkWid::handle_stdout);

    // 重新连接信号（确保只连接一次）
    connect(process, &QProcess::readyReadStandardError, [this]() {
        QString error = this->process->readAllStandardError();
        qDebug() << "Python Error:" << error;
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int code, QProcess::ExitStatus) {
                QString result = (code == 0 && allTestState == true)
                ? "✅ 测试成功"
                : "❌ 测试失败 (错误码: " + QString::number(code) + ")";

                ui->textEdit->append(result);

                if(code == 0 and arg == "0")
                    mCoreThread->setFlag(1);

                if(result == "✅ 测试成功" ) mPro->result = Test_Pass,mDev->dt.state = 1;
                else mPro->result = Test_Fail,mDev->dt.state = 0;

                //qDebug()<<code<<' '<<arg;
                mCoreThread->start();
                mCoreThread->setFlag(0);

                timer->stop();
                if(arg == "0"){
                    updateResult();
                }

                ui->startBtn->setText("开始测试");
                ui->pcbCode->clear();

                this->process->deleteLater();
                this->process = nullptr;
            });

    QStringList args;
    args << "-u"
         << scriptPath
         << digits
         << arg;

    process->start("python", args);

    if(arg == "0"){
        ui->startLab->setText(mDev->dt.date);
        ui->endLab->setText("---");
    }

    if (!process->waitForStarted(2000)) {
        ui->textEdit->append("⚠️ 无法启动进程：");
        ui->textEdit->append(process->errorString());
        process->deleteLater();
        process = nullptr;
    }
}

QString Home_WorkWid::getTime()
{
    QTime t(0,0,0,0);
    t = t.addSecs(mPro->startTime.secsTo(QTime::currentTime()));
    return  tr("%1").arg(t.toString("mm:ss"));
}

void Home_WorkWid::updateTime()
{
    QString str = getTime();
    QString style = "background-color:yellow; color:rgb(0, 0, 0);";
    style += "font:100 34pt \"微软雅黑\";";

    ui->timeLab->setText(str);
    ui->timeLab->setStyleSheet(style);
}

void Home_WorkWid::updateLcd(const QString &str)
{

    // if(mp[9].isNull())mp[8] = "4E:6B:2A:9C:1D:7F";
    // if(mp[10].isNull())mp[9] = "A1:B2:C3:D4:E5:F6";

    //更新测试数据
    ui->allLcd->display(ui->allLcd->value()+1);
   if(str == "Success")ui->okLcd->display(ui->okLcd->value()+1);
   else ui->errLcd->display(ui->errLcd->value()+1);

    //qDebug()<<"alllcd"<<ui->allLcd->value();
    ui->passLcd->display((ui->allLcd->value() == 0 ? 0 : ui->okLcd->value()/ui->allLcd->value()));


//    qDebug()<<str2<<' '<<str1;
}

void Home_WorkWid::intiTest()
{
    ui->textEdit->append("══════════════════════════");
    ui->textEdit->append(" 开始测试: " + ui->modeBox->currentText());
    ui->textEdit->append(" COM端口: " + digits);
    ui->startBtn->setText("测试中...");
    ui->textEdit->append("  正在测试,请勿关闭   \n");

    mp.clear();//清空mac信息
}

bool Home_WorkWid::validateComPort(const QString& comPort)
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

void Home_WorkWid::on_NoBtn_clicked()
{
    if (process && process->state() == QProcess::Running) {
        //qDebug() << "NO";
        ui->textEdit->append("否");
        process->write("N\n");
    } else {
       // qDebug() << "⚠️ 测试进程未启动，忽略 N 按钮操作";
        // 可选：提示用户或日志记录
        // QMessageBox::information(this, "提示", "测试尚未开始，无法发送 N 指令。");
    }
}

void Home_WorkWid::on_YesBtn_clicked()
{
    if (process && process->state() == QProcess::Running) {
       // qDebug() << "YES";

        ui->textEdit->append("是");
        process->write("Y\n");
    } else {
        //qDebug() << "⚠️ 测试进程未启动，忽略 Y 按钮操作";
        // 可选：提示用户或日志记录
        // QMessageBox::information(this, "提示", "测试尚未开始，无法发送 Y 指令。");
    }
}

void Home_WorkWid::handle_stdout()
{
    QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
    output.replace(QRegularExpression("\x1B\\[[0-?]*[ -/]*[@-~]"), ""); // 过滤 ANSI 转义

    QStringList lines = output.split('\n');
    foreach (const QString& line, lines) {
        if (line.trimmed().isEmpty()) continue;

        // 显示原始输出到 QTextEdit
        ui->textEdit->append("[输出] " + line.trimmed());

        if (line.contains("失败") || line.contains("在响应中找不到 IP 地址")) {
            allTestState = false;
            qDebug() << "检测到失败项:" << line.trimmed();
            mDev->dt.reason += "  " + line.trimmed();
        }

        // ========== 匹配网络接口（eth0: xxx 格式） ==========
        QRegularExpressionMatch ifaceMatch = ifaceRegex.match(line);
        if (ifaceMatch.hasMatch()) {
            QString iface = ifaceMatch.captured(1);
            QString value = ifaceMatch.captured(2);

            if (iface == "eth0")      ui->eth0Lab->setText(value),mp[0] = mDev->dt.eth1Mac = value;
            else if (iface == "eth1") ui->eth1Lab->setText(value),mp[1] = mDev->dt.eth2Mac = value;
            else if (iface == "eth2") ui->eth2Lab->setText(value),mp[2] = mDev->dt.eth3Mac = value;
            else if (iface == "spe0") ui->spe0Lab->setText(value),mp[3] = mDev->dt.spe1Mac = value;
            else if (iface == "spe1") ui->spe1Lab->setText(value),mp[4] = mDev->dt.spe2Mac = value;
            continue;
        }

        // ========== 匹配 SerialNumber=xxx ==========
        QRegularExpressionMatch serialMatch = serialRegex.match(line);
        if (serialMatch.hasMatch()) {
            QString serial = serialMatch.captured(1);
            ui->snLab->setText(serial),mp[7] = mDev->dt.sn = serial; // 需确保 UI 中有 serialLab 控件
            continue;
        }

        // ========== 匹配 HwRevision=xxx ==========
        QRegularExpressionMatch hwMatch = hwRegex.match(line);
        if (hwMatch.hasMatch()) {
            QString hw = hwMatch.captured(1);
            ui->hwLab->setText(hw),mp[5] = mDev->dt.hwVersion = hw;
            continue;
        }

        // ========== 匹配 ZB → 更新 zbLab（若输出含ZigBee信息） ==========
        QRegularExpressionMatch zbMatch = zbRegex.match(line);
        if (zbMatch.hasMatch()) {
            QString zb = zbMatch.captured(1);
            ui->zbLab->setText(zb),mp[9] = mDev->dt.zbMac = zb;
            continue;
        }

        // ========== 匹配 BT → 更新 btLab（若输出含蓝牙信息） ==========
        QRegularExpressionMatch btMatch = btRegex.match(line);
        if (btMatch.hasMatch()) {
            QString bt = btMatch.captured(1);
            ui->btLab->setText(bt),mp[8] = mDev->dt.btMac = bt;
            continue;
        }

        // ========== 匹配 FwRevision=xxx ==========
        QRegularExpressionMatch fwMatch = fwRegex.match(line);
        if (fwMatch.hasMatch()) {
            QString fw = fwMatch.captured(1);
            ui->fwLab->setText(fw),mp[6] = mDev->dt.fwVersion = fw;
            continue;
        }

    }

    QScrollBar* sb = ui->textEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void Home_WorkWid::updateResult()
{
    QString style;
    QString str = tr("---");
    if(Test_Fail == mPro->result) {
        str = tr("失败");
        style = "background-color:red; color:rgb(255, 255, 255);";
    } else {
        str = tr("成功");
        style = "background-color:green; color:rgb(255, 255, 255);";
    }
    style += "font:100 34pt \"微软雅黑\";";

    mPro->step = Test_End;
    ui->timeLab->setText(str);
    ui->timeLab->setStyleSheet(style);
    ui->startBtn->setText(tr("开始测试"));
    // QTimer::singleShot(450,this,SLOT(updateCntSlot()));
    str = QTime::currentTime().toString("hh:mm:ss");
    ui->endLab->setText(str);
}
