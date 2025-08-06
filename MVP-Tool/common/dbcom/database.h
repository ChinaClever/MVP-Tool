#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class DataBaseManager : public QObject
{
    Q_OBJECT
public:
    static DataBaseManager& instance(); // 单例
    QSqlDatabase& database();           // 获取连接
    bool init(const QString& dbName = "mydatabase.db"); // 初始化

private:
    explicit DataBaseManager(QObject *parent = nullptr);
    bool openDatabase(const QString& path);
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
