/*
 *
 *
 *  Created on: 2020年10月11日
 *      Author: Lzy
 */
#include "dblogs.h"

DbLogs::DbLogs()
{
    createTable();
    tableTile = tr("状态日志");
    //hiddens <<  9;
    headList << tr("序列号") << tr("硬件版本") << tr("固件版本") << tr("ETH1Mac") << tr("ETH2Mac") << tr("ETH3Mac")
             << tr("SPE1Mac") << tr("SPE2Mac") << tr("BTMac") << tr("ZBMac") << tr("pcb码") << tr("状态") << tr("原因");
}

void DbLogs::createTable()
{
    QString cmd =
        "CREATE TABLE IF NOT EXISTS %1 ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
        "date TEXT,"
        "time TEXT,"
        "sn TEXT,"
        "hw TEXT,"
        "fw TEXT,"
        "eth1 TEXT,"
        "eth2 TEXT,"
        "eth3 TEXT,"
        "spe1 TEXT,"
        "spe2 TEXT,"
        "bt TEXT,"
        "zb TEXT,"
        "pcbcode TEXT,"
        "state INTEGER,"
        "reason TEXT"
        ");";

    QSqlQuery query(mDb);
    if(!query.exec(cmd.arg(tableName()))) {
        throwError(query.lastError());
    }
}


DbLogs *DbLogs::bulid()
{
    static DbLogs* sington = nullptr;
    if(sington == nullptr)
        sington = new DbLogs();
    return sington;
}

bool DbLogs::insertItem(const sLogItem &item)
{
    QString cmd =
        "INSERT INTO %1 (date, time, sn, hw, fw, eth1, eth2, eth3, spe1, spe2, bt, zb, pcbcode, state, reason) "
        "VALUES (:date, :time, :sn, :hw, :fw, :eth1, :eth2, :eth3, :spe1, :spe2, :bt, :zb, :pcbcode, :state, :reason)";

    bool ret = modifyItem(item, cmd.arg(tableName()));
    if (ret) emit itemChanged(item.id, Insert);
    return ret;
}

bool DbLogs::modifyItem(const sLogItem &item, const QString &cmd)
{
    QSqlQuery query(mDb);
    query.prepare(cmd);

    query.bindValue(":date", item.date);
    query.bindValue(":time", item.time);
    query.bindValue(":hw", item.hw);
    query.bindValue(":fw", item.fw);
    query.bindValue(":eth1", item.eth1Mac);
    query.bindValue(":eth2", item.eth2Mac);
    query.bindValue(":eth3", item.eth3Mac);
    query.bindValue(":spe1", item.spe1Mac);
    query.bindValue(":spe2", item.spe2Mac);
    query.bindValue(":bt", item.btMac);
    query.bindValue(":zb", item.zbMac);
    query.bindValue(":pcbcode", item.pcbCode);
    query.bindValue(":state", item.state ? 1 : 0); // bool 转 int
    query.bindValue(":reason", item.result);
    query.bindValue(":sn", item.sn);

    bool ret = query.exec();
    if (!ret) throwError(query.lastError());
    return ret;
}

