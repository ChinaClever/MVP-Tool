/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "test_corethread.h"
#include "MainPage/createini.h"
#include "baselogs.h"
Test_CoreThread::Test_CoreThread(QObject *parent) : BaseThread(parent)
{

}

void Test_CoreThread::initFunSlot()
{

}


void Test_CoreThread::workDown()
{   /*
    bool ret = programFab(1);
    if(ret) {
        ret =  waitFor();
        if(ret) ret = mNetWork->startProcess();
        if(ret) ret = macSnCheck();
        //if(ret) ret = printer();
    }*/
}

void Test_CoreThread::workResult()
{
    BaseLogs *logs = BaseLogs::bulid();
    bool res = logs->setLogs(mDev->dt);

    qDebug()<<"res: "<<res;
}

void Test_CoreThread::run()
{
    if (isRun) return;
    isRun = true;
    QString result;

    if(flag == 1) {
        // 先执行 PrintLabel1
        result = createIni::toIni1(&mDev->dt);
        emit updateLcd(result);
        // 再执行 PrintLabel2
        result = createIni::toIni2(&mDev->dt);
        emit updateLcd(result);
        // 最后执行 WriteLog
        workResult();
    } else {
        // 只执行 WriteLog
        workResult();
    }

    //emit taskFinished(currentTask, result);

    isRun = false;
}
