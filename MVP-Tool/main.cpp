#include "mainwindow.h"
#include "database.h"
#include "dbuser.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(!DataBaseManager::instance().init("MvpTool.db")){
        qCritical()<<"数据库初始化失败";
        return -1;
    }

    DbUser::build();

    MainWindow w;
    w.show();
    return a.exec();
}
