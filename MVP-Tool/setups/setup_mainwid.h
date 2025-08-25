#ifndef SETUP_MAINWID_H
#define SETUP_MAINWID_H
#include "serialstatuswid.h"
#include <QWidget>
#include "usermainwid.h"
#include "flash/flash_mainwid.h"

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
protected:
    void initLogCount();

private:
    Ui::Setup_MainWid *ui;
    SerialStatusWid* mDeWid;
    UserMainWid *mUserWid;
    Flash_MainWid *mFlashWid;
};

#endif // SETUP_MAINWID_H
