#ifndef SETUP_MAINWID_H
#define SETUP_MAINWID_H
#include "serialstatuswid.h"
#include <QWidget>

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

private:
    Ui::Setup_MainWid *ui;
    SerialStatusWid* mDeWid;
};

#endif // SETUP_MAINWID_H
