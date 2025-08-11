/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "test_serialnumber.h"

Test_SerialNumber::Test_SerialNumber(QObject *parent) : BaseThread(parent)
{

}

Test_SerialNumber *Test_SerialNumber::bulid(QObject *parent)
{
    static Test_SerialNumber* sington = nullptr;
    if(sington == nullptr)
        sington = new Test_SerialNumber(parent);
    return sington;
}

QString Test_SerialNumber::getSn()
{
    QString cmd = "2I3";
    mItem->currentNum +=1;
    int m = QDate::currentDate().month();
    int y = QDate::currentDate().year() - 2020;
    for(int i=0; i<3; ++i) cmd += "%" + QString::number(i+1);
    QString sn  = QString(cmd).arg(m, 1, 16).arg(y)
            .arg(mItem->currentNum, 5, 10, QLatin1Char('0'));

    Cfg::bulid()->setCurrentNum();
    return sn.toUpper();
}

QString Test_SerialNumber::updateMacAddr(int step)
{
    sMac *it = &(mItem->macs);
    if(it->mac.size() > 5) {
        MacAddr *mac = MacAddr::bulid();
        it->mac = mac->macAdd(it->mac, step);
        BaseLogs::bulid()->writeMac(it->mac);
        Cfg::bulid()->write("mac", it->mac, "Mac");
    } else {
        qDebug() << "updateMacAddr err" << it->mac;
    }

    return it->mac;
}

void Test_SerialNumber::createSn()
{
    mDt->sn = getSn();
    updateMacAddr();
}
