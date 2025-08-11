/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "baselogs.h"
extern QString user_land_name();

BaseLogs::BaseLogs(QObject *parent) : QThread(parent)
{

}

BaseLogs::~BaseLogs()
{
    wait();
}

BaseLogs *BaseLogs::bulid(QObject *parent)
{
    static BaseLogs* sington = nullptr;
    if(sington == nullptr)
        sington = new BaseLogs(parent);
    return sington;
}


bool BaseLogs::writeMac()
{
    sMacItem it;

    it.fw = mDt->fwVersion;
    //it.user = mItem->user;
    it.sn = mDt->sn;
    it.mac = mMac;
    mMac.clear();

    return DbMacs::bulid()->insertItem(it);
}
bool BaseLogs::appendLogItem(const QString &str, bool pass)
{
    sStateItem it;
    if(pass) {
        it.result = tr("通过");
    } else {
        it.result = tr("失败");
    }

    it.memo = str;
    mLogItems << it;

    return pass;
}

bool BaseLogs::setLogs(sDevInfo& dt)
{
    Db_Tran db;
    sLogItem it;

    it.fw = dt.fwVersion;
    it.hw = dt.hwVersion;
    it.result = dt.reason;
    it.sn = dt.sn;
    it.eth1Mac = dt.eth1Mac;
    it.eth2Mac = dt.eth2Mac;
    it.eth3Mac = dt.eth3Mac;
    it.spe1Mac = dt.spe1Mac;
    it.spe2Mac = dt.spe2Mac;
    it.btMac = dt.btMac;
    it.zbMac = dt.zbMac;
    it.pcbCode = dt.pcbCode;
    it.state = dt.state;
    return DbLogs::bulid()->insertItem(it);
}

void BaseLogs::saveLogs()
{
    bool ret = writeLog();
    if(ret) {
        writeLogs();
        if(mMac.size()) writeMac();
    } else {
        // updatePro(tr("因未创建序列号，日志无法保存！"), false);
    }
    mLogItems.clear();
}

bool BaseLogs::writeLog()
{
    Db_Tran db;
    sLogItem it;



    return DbLogs::bulid()->insertItem(it);
}

bool BaseLogs::initItem(sStateItem &it)
{


    return it.sn.size();
}

void BaseLogs::writeLogs()
{
    Db_Tran db;
    for(int i=0; i<mLogItems.size(); ++i) {
        sStateItem it = mLogItems.at(i);
        if(initItem(it)) DbStates::bulid()->insertItem(it);
    }
    mLogItems.clear();
}
