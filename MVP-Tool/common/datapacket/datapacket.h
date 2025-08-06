#ifndef DATAPACKET_H
#define DATAPACKET_H
#include <QColor>
#include <QtCore>
struct sDevInfo
{
    QString sn;                // 序列号
    QString hwVersion;         // 硬件版本
    QString fwVersion;         // 固件版本
    QString date;              // 日期
    QString eth1Mac;           // ETH1 Mac 地址
    QString eth2Mac;           // ETH2 Mac 地址
    QString eth3Mac;           // ETH3 Mac 地址
    QString spe1Mac;           // SPE1 Mac 地址
    QString spe2Mac;           // SPE2 Mac 地址
    QString btMac;             // BT Mac 地址
    QString zbMac;             // ZB Mac 地址
    QString pcbCode;           // PCB 码
    QString reason;            // 原因

    bool state;             // 状态
};

struct sDevData
{
    sDevInfo dt; //设备类型
};

enum {
    Test_Fun, // 功能
    Test_Start, // 开始
    Test_Set,
    Test_Secure,
    Test_Collect,
    Test_raritan,
    Test_Over, // 终止
    Test_End, // 完成

    Test_Info=0,
    Test_Pass=1,
    Test_Fail=2,
};

struct sProgress
{
    sProgress() {step=0;}

    uchar step; // 步骤
    QString time;
    QList<bool> pass;
    QStringList status;

    uchar result;    // 最终结果
    QTime startTime;

    bool allTest;
};

class sDataPacket
{
    sDataPacket();
public:
    static sDataPacket *build();

    void init();
    sDevData *getDev() {return dev;}
    sProgress *getPro() {return pro;}

private:
    sDevData *dev;
    sProgress *pro;
};

#endif // DATAPACKET_H
