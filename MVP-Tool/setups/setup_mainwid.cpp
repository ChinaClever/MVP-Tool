#include "setup_mainwid.h"
#include "ui_setup_mainwid.h"

Setup_MainWid::Setup_MainWid(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Setup_MainWid)
{
    ui->setupUi(this);
    intiSerial();
        groupBox_background_icon(this);
    mUserWid = new UserMainWid(ui->stackedWid);
    ui->stackedWid->addWidget(mUserWid);
    initLogCount();
}

void Setup_MainWid::intiSerial()
{
    mDeWid = new SerialStatusWid(ui->deWid);
    //mItem->deSerial = mDeWid->initSerialPort(tr("Debug"));
}

void Setup_MainWid::initLogCount()
{
    Cfg *con = Cfg::bulid();
    int value = con->read("log_count", 10, "Sys").toInt();

    sCfgItem *item = con->item;
    item->logCount = value * 10000;
    ui->logCountSpin->setValue(value);
}

Setup_MainWid::~Setup_MainWid()
{
    delete ui;
}
