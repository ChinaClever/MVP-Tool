/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "test_network.h"

Test_NetWork::Test_NetWork(QObject *parent) : BaseThread(parent)
{

}

Test_NetWork *Test_NetWork::bulid(QObject *parent)
{
    static Test_NetWork* sington = nullptr;
    if(sington == nullptr)
        sington = new Test_NetWork(parent);
    return sington;
}

void Test_NetWork::initFunSlot()
{    
    mProcess = new QProcess(this);
    //mProcess->setProcessChannelMode(QProcess::SeparateChannels);
    //mProcess->setReadChannel(QProcess::StandardOutput);

    mUdp = new UdpRecvSocket(this);
    mUdp->initSocket(12306);
    this->start();
}

bool Test_NetWork::checkNet()
{    
    bool ret = true;
    QString ip = mDt->ip;
    updatePro(tr("检测设备网络通讯 "));
    for(int i=0; i<2; ++i) {
        ret = cm_pingNet(ip);
        if(ret) break; else delay(3);
    }

    QString str = tr("Ping ") +ip;
    if(ret) str += tr(" 正常"); else str += tr(" 错误");
    return updatePro(str, ret);
}

QStringList Test_NetWork::getCmd()
{
    mProcess->close();
    QStringList arguments;
    arguments << mDt->user << mDt->pwd;
    if(!mDt->aiFind) arguments << mDt->ip;

    return arguments;
}

QString Test_NetWork::getExeFile()
{
    QString fn = "py_fvt_node";
#if defined(Q_OS_WIN32)
    fn += ".exe";
#endif

    QFile file(fn);
    if (file.exists()){
        updatePro(tr("正在启动测试脚本"));
    } else {
        updatePro(tr("启动测试脚本 %1 失败").arg(fn), false);
        fn.clear();
    }

    return fn;
}

bool Test_NetWork::startProcess()
{
    bool ret = mDt->aiFind;
    if(!ret) ret = checkNet();
    if(ret) {
        QString fn = getExeFile();
        if(!fn.size()) return false;
        QStringList cmd = getCmd();
        mProcess->start(fn, cmd);

        ret = mProcess->waitForFinished(5*60*1000);
        QByteArray bs = mProcess->readAllStandardOutput();
        bs += mProcess->readAllStandardError();
        QString str = QString::fromLocal8Bit(bs); emit msgSig(str);
        if(str.contains("'str' object is not callable")) {ret = false;}
    }

    return ret;
}


void Test_NetWork::pduInfo(int fn, QString &msg)
{
    switch (fn) {
    case 1: mDt->ctrlBoardSerial = msg; break;
    case 2: mDt->macAddress = msg.toUpper(); break;
    case 3: mDt->hwRevision = msg; break;
    case 4: mDt->fwRevision = msg; break;
    case 5: mDt->serialNumber = msg; break;
    case 6: mDt->manufacturer = msg; break;
    case 7: mDt->model = msg; break;
    }
}

void Test_NetWork::workDown()
{
    UdpBaseData *res = mUdp->getData();
    if(res) {
        QStringList list = QString(res->datagram).split(";");
        if(list.size() == 3) {
            int fn = list.first().toInt();
            QString str = list.at(1);
            bool pass = list.last().toInt();
            if(fn) {
                pduInfo(fn, str);
            } else {
                updatePro(str, pass, 0);
            }
        } else {
            qDebug() <<"Test_NetWork workDown err" << list;
        }
        delete res;
    } else {
        msleep(1);
    }
}

void Test_NetWork::run()
{
    isRun = true;
    while (isRun) {
        workDown();
    }
}
