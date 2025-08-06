#include "setup_mainwid.h"
#include "ui_setup_mainwid.h"

Setup_MainWid::Setup_MainWid(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Setup_MainWid)
{
    ui->setupUi(this);
    intiSerial();
}

void Setup_MainWid::intiSerial()
{
    mDeWid = new SerialStatusWid(ui->deWid);
    //mItem->deSerial = mDeWid->initSerialPort(tr("Debug"));
}

Setup_MainWid::~Setup_MainWid()
{
    delete ui;
}
