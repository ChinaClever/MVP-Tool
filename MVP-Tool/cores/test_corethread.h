#ifndef TEST_CORETHREAD_H
#define TEST_CORETHREAD_H
#include "baseobject.h"


class Test_CoreThread : public BaseThread
{
    Q_OBJECT
public:
    enum TaskType {
        PrintLabel1,
        PrintLabel2,
        WriteLog
    };

    explicit Test_CoreThread(QObject *parent = nullptr);

    bool isContinue;
    void setTaskType(TaskType type) { currentTask = type; }
    void setFlag(bool x){flag = x;}
signals:
    void waitSig();
    void updateLcd(const QString &message);

protected:
    void run();
    void workDown();
    void workResult();

protected slots:
    void initFunSlot();

private:
    TaskType currentTask = PrintLabel1;
    bool flag;

};

#endif // TEST_CORETHREAD_H
