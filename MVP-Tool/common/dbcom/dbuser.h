#ifndef DB_USR_H
#define DB_USR_H
#include "basicsql.h"

struct sUserItem : public DbBasicItem{
    sUserItem():jur(0){}
    int jur;
    QString name,pwd,jurisdiction, email,telephone,remarks;
};



class DbUser : public SqlBasic<sUserItem>
{
    Q_OBJECT
    DbUser();
public:
    static DbUser* build();
    QString tableName(){return "users";}

    int contains(const QString &name);
    bool insertItem(sUserItem& item); //增加
    bool updateItem(const sUserItem& item); //修改

    sUserItem selItemsByName(const QString& Name);  //查询
    void removeItemsByName(const QString& name);    //删除

    void createTable();


protected:
    bool modifyItem(const sUserItem& item,const QString& cmd);
    void selectItem(QSqlQuery &query,sUserItem &item);
};

#endif // DB_USR_H
