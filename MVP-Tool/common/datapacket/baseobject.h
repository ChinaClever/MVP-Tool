#ifndef BASEOBJECT_H
#define BASEOBJECT_H
#include "datapacket.h"
class BaseObject
{
public:
    BaseObject();
    void init() {mPacket->init();}
    //void intiPro(){mPro->init();}
protected:
    sDevInfo *mDt;
    sDevData *mDev;

    sProgress *mPro;
    // sDevObj *mData;
    sDataPacket *mPacket;
};

class BaseThread : public QThread, public BaseObject
{
    Q_OBJECT
public:
    explicit BaseThread(QObject *parent = nullptr);
    ~BaseThread();

    bool updatePro(const QString &str, bool pass=true, int sec=0);
    bool delay(int s=1) {return mdelay(6*s);}
    bool mdelay(int s=1);

protected slots:
    virtual void initFunSlot();

protected:
    bool isRun;

};

#endif // BASEOBJECT_H
