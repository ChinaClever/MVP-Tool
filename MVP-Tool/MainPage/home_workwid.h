#ifndef HOME_WORKWID_H
#define HOME_WORKWID_H
#include "metadatastruct.h"
#include <QWidget>
#include <QProcess>
#include <baseobject.h>
#include "test_corethread.h"
namespace Ui {
class Home_WorkWid;
}

class Home_WorkWid : public QWidget,public BaseObject
{
    Q_OBJECT

public:
    explicit Home_WorkWid(QWidget *parent = nullptr);
    bool pcbCheck();
    ~Home_WorkWid();

protected:
    QString getTime();

private slots:
    void updateTime();
    void on_startBtn_clicked();
    bool intiarg(); //初始化参数
    void intiTest();   //初始化输出框
    void workProcess();
    void initFunSlot();
    void updateResult();
    void handle_stdout();  // 处理标准输出
 //   void saveCurrentData(); // 新增保存按钮槽函数

    void on_NoBtn_clicked();
    void on_YesBtn_clicked();
    void uiClear();
    void updateLcd(const QString &message);

private:
    Ui::Home_WorkWid *ui;

    InterfaceInfo infoData;

    QString scriptPath;
    QString arg;
    QString digits;
    QString pythonInterpreter;
    QProcess *process; //运行脚本进程
    bool isCheck;
    QTimer *timer;
    bool validateComPort(const QString& comPort);
    Test_CoreThread *mCoreThread;
};

#endif // HOME_WORKWID_H
