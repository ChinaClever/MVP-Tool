#ifndef DBLOGS_H
#define DBLOGS_H
#include "basicsql.h"

struct sLogItem : public DbBasicItem{
    sLogItem(){}
    QString user, fw, hw, result, sn;

    QString eth1Mac;           // ETH1 Mac 地址
    QString eth2Mac;           // ETH2 Mac 地址
    QString eth3Mac;           // ETH3 Mac 地址
    QString spe1Mac;           // SPE1 Mac 地址
    QString spe2Mac;           // SPE2 Mac 地址
    QString btMac;             // BT Mac 地址
    QString zbMac;             // ZB Mac 地址
    QString pcbCode;           // PCB 码

    bool state;             // 状态
};

class DbLogs : public SqlBasic<sLogItem>
{
    DbLogs();
public:
    static DbLogs* bulid();
    QString tableName(){return "logs";}
    bool insertItem(const sLogItem& item);

protected:
    void createTable();
    bool modifyItem(const sLogItem& item,const QString& cmd);
};

#endif // DBLOGS_H
