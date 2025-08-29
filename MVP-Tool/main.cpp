#include "mainwindow.h"
#include "dbuser.h"

#include <QApplication>
#include "qtsingleapplication.h"
int main(int argc, char *argv[])
{
    QtSingleApplication a(QLatin1String("MVP3"),argc, argv);
    if(a.isRunning()) {
        MsgBox::critical(NULL, QObject::tr("MVP3测试程序已运行！"));
        a.sendMessage("raise_window_noop", 1000); //4s后激活前个实例
        return EXIT_SUCCESS;
    }
    MainWindow w;
    w.show();
    return a.exec();
}
