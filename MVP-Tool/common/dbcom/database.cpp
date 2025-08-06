#include "database.h"
#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QDebug>

DataBaseManager::DataBaseManager(QObject *parent)
    : QObject(parent)
{
    // 改为带名字的连接
    if (QSqlDatabase::contains("MvpToolConnection")) {
        m_db = QSqlDatabase::database("MvpToolConnection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "MvpToolConnection");
    }
}

DataBaseManager& DataBaseManager::instance()
{
    static DataBaseManager inst;
    return inst;
}

bool DataBaseManager::init(const QString& dbName)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString dbDir = appDir + "/db";

    QDir dir;
    if (!dir.exists(dbDir)) {
        if (!dir.mkpath(dbDir)) {
            qWarning() << "无法创建数据库目录:" << dbDir;
            return false;
        }
    }

    QString dbPath = dbDir + "/" + dbName;
    return openDatabase(dbPath);
}

bool DataBaseManager::openDatabase(const QString& path)
{
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        qCritical() << "数据库打开失败:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "数据库已连接:" << path;
    return true;
}

QSqlDatabase& DataBaseManager::database()
{
    return m_db;
}
