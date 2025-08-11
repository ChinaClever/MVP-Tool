#ifndef TEST_NETWORK_H
#define TEST_NETWORK_H
#include "test_fabpartition.h"

class Test_NetWork : public BaseThread
{
    Q_OBJECT
    explicit Test_NetWork(QObject *parent = nullptr);
public:
    static Test_NetWork *bulid(QObject *parent = nullptr);

    bool startProcess();

signals:
    void msgSig(QString str);

protected:
    void run();
    void workDown();
    bool checkNet();
    QString getExeFile();
    void pduInfo(int fn, QString &msg);

    QStringList getCmd();

protected slots:
    void initFunSlot();

private:
    UdpRecvSocket *mUdp;
    QProcess *mProcess;
};

#endif // TEST_NETWORK_H
