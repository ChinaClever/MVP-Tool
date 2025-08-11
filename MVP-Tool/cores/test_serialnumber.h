#ifndef TEST_SERIALNUMBER_H
#define TEST_SERIALNUMBER_H
#include "baselogs.h"
// #include "udprecvsocket.h"

class Test_SerialNumber : public BaseThread
{
    Q_OBJECT
    explicit Test_SerialNumber(QObject *parent = nullptr);
public:
    static Test_SerialNumber *bulid(QObject *parent = nullptr);
    void createSn();
    QString updateMacAddr(int step=1);

protected:
    QString getSn();
};

#endif // TEST_SERIALNUMBER_H
