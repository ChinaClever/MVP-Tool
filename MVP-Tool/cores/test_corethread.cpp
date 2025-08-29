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

    mFab = Test_Fabpartition::build(this);
}

void Test_CoreThread::initFunSlot()
{
}


void Test_CoreThread::workDown()
{
    if(flag) {
        QString result;
        // 先执行 PrintLabel1
        result = createIni::toIni1(&mDev->dt); //大标签
        emit updateLcd(result);
        // 再执行 PrintLabel2
        QVector<QString>macs;
        macs.push_back(mDev->dt.btMac);
        macs.push_back(mDev->dt.eth1Mac);
        macs.push_back(mDev->dt.eth2Mac);
        macs.push_back(mDev->dt.eth3Mac);
        macs.push_back(mDev->dt.spe1Mac);
        macs.push_back(mDev->dt.spe2Mac);
        macs.push_back(mDev->dt.zbMac);


        for(int i = 0; i < 1; i ++ ){

            result = createIni::toIni2(&mDev->dt,macs[i]);
        }
        emit updateLcd(result);
        // 最后执行 WriteLog
    }

    workResult();
    //emit taskFinished(currentTask, result);
}

void Test_CoreThread::workResult()
{
    BaseLogs *logs = BaseLogs::bulid();
    bool res = logs->setLogs(mDev->dt);

    qDebug()<<"res: "<<res;
}

bool Test_CoreThread::initFun()
{
    bool ret = updatePro(tr("即将开始"));
    return  ret;
}

bool Test_CoreThread::programFab()
{
    bool ret = mFab->check();
    if(ret){
        if(mDt->img.size()){
            //ret = mFab->programFull();
        }

        ret = mFab->workDown();
    }
    return ret;
}

void Test_CoreThread::run()
{
    if (isRun) return;
    isRun = true;

    switch(mPro->step){
        case Test_Start : workDown();break;
        case Test_Set : programFab();break;
    }


    isRun = false;
}
