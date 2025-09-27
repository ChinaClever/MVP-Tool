#include "home_workwid.h"
#include "ui_home_workwid.h"
#include "common/globals/globals.h"
#include "backcolour/backcolourcom.h"
#include "config.h"
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
    initLcdNum();
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
    delete ui;
}

bool Home_WorkWid::intiarg()
{
    pythonInterpreter = "python";
    arg = QString::number(ui->modeBox->currentIndex());

    QString dir = QDir::currentPath();

    scriptPath = dir + "/MVP3_JSON_PRC_With_SecureBoot.exe";
    //scriptPath = "D:/GitHub/MVP-Tool/MVP-Tool/pdu-python-api-040350-51598/dist/MVP3_JSON_PRC_With_SecureBoot.exe";
    //scriptPath = "D:/GitHub/MVP-Tool/MVP-Tool/pdu-python-api-040350-51598/MVP3_JSON_PRC_With_SecureBoot.py";

    if (!QFile::exists(scriptPath)) {
        qDebug()<<scriptPath;
        QMessageBox::critical(this, "错误", "找不到测试脚本: MVP3_JSON_PRC_With_SecureBoot.exe");
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
    ui->PCBLab->setText("--- ---");
    ui->btLab->setText("--- ---");
    ui->fwLab->setText("--- ---");
    ui->snLab->setText("--- ---");
    ui->spe0Lab->setText("--- ---");
    ui->zbLab->setText("--- ---");
    ui->eth0Lab->setText("--- ---");
    ui->eth2Lab->setText("--- ---");
    ui->textEdit->clear();
}

void Home_WorkWid::on_startBtn_clicked()
{
    if(ui->startBtn->text() == "开始测试"){
        mCoreThread->setFlag(0);
        mPacket->init();
        //if(!pcbCheck())return ;
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
            updateResult();
        }
    }
}

void Home_WorkWid::initFunSlot()
{
    Cfg *cfg = Cfg::bulid();
    QString fw = cfg->getFwVersion();   // 或者 cfg->read("Fw", "", "FwVersion").toString();
    ui->LineFwVersion->setText(fw);
    ui->LineFwVersion->setEnabled(false);
    mPro->step = Test_End;
}

void Home_WorkWid::initLcdNum()
{
    Cfg *con = Cfg::bulid();

    con->initCnt();

    ui->passLcd->display(con->item->cnt.cnt);
    ui->allLcd->display(con->item->cnt.all);
    ui->okLcd->display(con->item->cnt.ok);
    ui->errLcd->display(con->item->cnt.err);
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
    connect(process, &QProcess::readyReadStandardOutput, this, &Home_WorkWid::handle_stdout);
    connect(process, &QProcess::readyReadStandardError, [this]() {
        QString error = this->process->readAllStandardError();
        qDebug() << "Python Error:" << error;
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int code, QProcess::ExitStatus) {

                checkLab();
                QString result = (code == 0 && allTestState == true)
                ? "✔ 测试成功"
                : "❌ 测试失败 (错误码: " + QString::number(code) + ")";

                ui->textEdit->append(result);

                if(code == 0 and arg == "0" && allTestState == true)
                    mCoreThread->setFlag(1);

                if(result == "✔ 测试成功" ) mPro->result = Test_Pass,mDev->dt.state = 1;
                else mPro->result = Test_Fail,mDev->dt.state = 0;

#if DeBugMode
                qDebug()<<"test";   mCoreThread->setFlag(1);
#endif
                mCoreThread->start();

                timer->stop();
                if(arg == "0"){
                    updateResult();
                }
                ui->startBtn->setText("开始测试");
                this->process->deleteLater();
                this->process = nullptr;
            });
    QStringList args;
    args << "-u"
         << scriptPath;
         //<< digits
        // << arg;
    QString exePath = scriptPath;
    //process->start("python", args);
    process->start(exePath);

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

void Home_WorkWid::checkLab()
{
    QList<QPair<QLabel*, QString>> labels = {{ui->snLab, "sn"},{ui->PCBLab,  "PCB"},{ui->eth0Lab, "eth0"},
                                              {ui->eth1Lab, "eth1"},{ui->eth2Lab, "eth2"},{ui->spe0Lab, "spe0"},
                                              {ui->spe1Lab, "spe1"},{ui->btLab,"bt"},{ui->zbLab,"zb"},
                                              {ui->fwLab,"fw"},{ui->hwLab, "hw"} };
    for (auto &pair : labels) {
        QLabel *lab = pair.first;
        QString name = pair.second;
        if (lab->text().isEmpty() || lab->text() == "--- ---") {
            qWarning() << "检测失败:" << name << "为空";
            mDev->dt.reason += "缺少" + name + " ";
            allTestState = false;
        }
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
    QString style = "background-color:yellow; color:rgb(0, 0, 0);";style += "font:100 34pt \"微软雅黑\";";
    ui->timeLab->setText(str);
    ui->timeLab->setStyleSheet(style);
}

void Home_WorkWid::updateLcd(const QString &str)
{
    ui->allLcd->display(ui->allLcd->value()+1);
    if(str == "Success")ui->okLcd->display(ui->okLcd->value()+1);
    else ui->errLcd->display(ui->errLcd->value()+1);
    ui->passLcd->display((ui->allLcd->value() == 0 ? 0 : ui->okLcd->value()/ui->allLcd->value()));

    Cfg *con = Cfg::bulid();


    con->item->cnt.all = ui->allLcd->value();
    con->item->cnt.cnt = ui->passLcd->value();
    con->item->cnt.err = ui->errLcd->value();
    con->item->cnt.ok = ui->okLcd->value();

    con->writeCnt();

}

void Home_WorkWid::intiTest()
{
    ui->textEdit->append("══════════════════════════");
    ui->textEdit->append(" 开始测试: " + ui->modeBox->currentText());
    ui->startBtn->setText("测试中...");ui->textEdit->append("  正在测试,请勿关闭   \n");
}

bool Home_WorkWid::validateComPort(const QString& comPort)
{
    if(comPort.isEmpty()) {
        QMessageBox::critical(this, "错误", "COM端口号不能为空");
        return false;
    }
    return true;
}

void Home_WorkWid::checkMac(int tp, const QString &mac)
{
    QString cleanMac = mac;
    cleanMac.remove(QRegularExpression("[:-]")).toUpper();

    QByteArray macBytes = QByteArray::fromHex(cleanMac.toLatin1());

    QByteArray start, end;
    if (tp == 0) { // 普通 MAC
        start = QByteArray::fromHex("0004742D0100");
        end   = QByteArray::fromHex("0004742DFFFF");
    }
    else if (tp == 1) { // Zigbee
        start = QByteArray::fromHex("000474000110A040");
        end   = QByteArray::fromHex("000474000110FFFF");
    }
    else {
        return;
    }

    // 判断是否在范围内
    if (macBytes < start || macBytes > end) {
        ui->textEdit->append("MAC 地址不在范围内！");
        allTestState = false;
        // mDev->dt.reason += " MAC 地址不在范围内";
    } else {
        ui->textEdit->append("MAC 地址合格。");
    }
}


void Home_WorkWid::checkPn(const QString &sn)
{
    if (sn.length() != 11) {
        ui->textEdit->append("SN码长度应为11位！");
        allTestState = false;
        mDev->dt.reason += " SN码长度不合格";
        return;
    }

    QString expectedFactor = "048";
    QString expectedTp = "2";
    QString expectedBench = "01";
    QString expectedFree = "00";

    QString factor = sn.mid(0, 3);
    QString tp = sn.mid(3, 1);
    QString bench = sn.mid(4, 2);
    QString free = sn.mid(9, 2);

    // 检查固定部分是否都符合
    if (factor != expectedFactor || tp != expectedTp || bench != expectedBench || free != expectedFree) {
        ui->textEdit->append("SN码固定部分不合格！");
        allTestState = false;
        mDev->dt.reason += " SN码固定部分不合格";
        return;
    }
    ui->textEdit->append("SN码符合标准范围");
}



void Home_WorkWid::on_NoBtn_clicked()
{
    if (process && process->state() == QProcess::Running) {
        ui->textEdit->append("否");process->write("N\n");
    }
}

void Home_WorkWid::on_YesBtn_clicked()
{
    if (process && process->state() == QProcess::Running) {
        ui->textEdit->append("是");process->write("Y\n");
    }
}

void Home_WorkWid::handle_stdout()
{
    QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
    output.replace(QRegularExpression("\x1B\\[[0-?]*[ -/]*[@-~]"), "");
    QStringList lines = output.split('\n');
    foreach (const QString& line, lines) {
        if (line.trimmed().isEmpty()) continue;
        ui->textEdit->append("[输出] " + line.trimmed());
        if (line.contains("失败") || line.contains("在响应中找不到 IP 地址")) {
            allTestState = false;
            qDebug() << "检测到失败项:" << line.trimmed();
            mDev->dt.reason += "  " + line.trimmed();
        }
        QRegularExpressionMatch ifaceMatch = ifaceRegex.match(line);
        if (ifaceMatch.hasMatch()) {
            QString iface = ifaceMatch.captured(1);
            QString value = ifaceMatch.captured(2);
            if (iface == "eth0")      ui->eth0Lab->setText(value),mDev->dt.eth1Mac = value;
            else if (iface == "eth1") ui->eth1Lab->setText(value),mDev->dt.eth2Mac = value;
            else if (iface == "eth2") ui->eth2Lab->setText(value),mDev->dt.eth3Mac = value;
            else if (iface == "spe0") ui->spe0Lab->setText(value),mDev->dt.spe1Mac = value;
            else if (iface == "spe1") ui->spe1Lab->setText(value),mDev->dt.spe2Mac = value;

            checkMac(0,value);
            continue;
        }
        QRegularExpressionMatch serialMatch = serialRegex.match(line);
        if (serialMatch.hasMatch()) {
            QString serial = serialMatch.captured(1);
            ui->snLab->setText(serial);
            mDev->dt.sn = serial;
            checkPn(serial);
            continue;
        }
        QRegularExpressionMatch boardMatch = boardRegex.match(line);
        if (boardMatch.hasMatch()) {
            QString value = boardMatch.captured(1);
            ui->PCBLab->setText(value);mDev->dt.pcbCode = value;
            continue;
        }
        QRegularExpressionMatch zbMatch = zbRegex.match(line);
        if (zbMatch.hasMatch()) {
            QString zb = zbMatch.captured(1);
            ui->zbLab->setText(zb),mDev->dt.zbMac = zb;
            checkMac(1,zb);
            continue;
        }
        QRegularExpressionMatch btMatch = btRegex.match(line);
        if (btMatch.hasMatch()) {
            QString bt = btMatch.captured(1);
            ui->btLab->setText(bt),mDev->dt.btMac = bt;
            checkMac(0,bt);
            continue;
        }
        QRegularExpressionMatch fwMatch = fwRegex.match(line);
        if (fwMatch.hasMatch()) {
            QString fw = fwMatch.captured(1);
            ui->fwLab->setText(fw),mDev->dt.fwVersion = fw;

            if(fw != ui->LineFwVersion->text()){
                allTestState = false;
                ui->textEdit->append("固件版本错误，请检查修改固件版本 !");
            }

            continue;
        }
        QRegularExpressionMatch hwMatch = hwRegex.match(line);
        if (hwMatch.hasMatch()) {
            QString hw = hwMatch.captured(1);
            ui->hwLab->setText(hw),mDev->dt.hwVersion = hw;
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
    str = QTime::currentTime().toString("hh:mm:ss");
    ui->endLab->setText(str);
}

void Home_WorkWid::on_ReviseBtn_clicked()
{
    if (ui->ReviseBtn->text() == "修改") {
        ui->ReviseBtn->setText("保存");
        ui->LineFwVersion->setEnabled(true);
    } else {
        ui->ReviseBtn->setText("修改");
        ui->LineFwVersion->setEnabled(false);

        // 保存新版本号
        QString fw = ui->LineFwVersion->text();
        Cfg *cfg = Cfg::bulid();
        cfg->writeFwVersion(fw);
    }
}

