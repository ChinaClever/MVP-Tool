#ifndef SETUP_MAINWID_H
#define SETUP_MAINWID_H
#include "serialstatuswid.h"
#include <QWidget>
#include "usermainwid.h"
#include "deviceidgenerator.h"
#include "test_fabpartition.h"
namespace Ui {
class Setup_MainWid;
}

class Setup_MainWid : public QWidget
{
    Q_OBJECT

public:
    explicit Setup_MainWid(QWidget *parent = nullptr);
    ~Setup_MainWid();

    void intiSerial();
public slots:
    void renewMacSlot();
protected:
    void initLogCount();

private:
    DeviceIdGenerator *gen;
    Ui::Setup_MainWid *ui;
    SerialStatusWid* mDeWid;
    UserMainWid *mUserWid;
    Test_Fabpartition *mFab;
};

#endif // SETUP_MAINWID_H
