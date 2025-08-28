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
    gen = &DeviceIdGenerator::instance();
    initLogCount();
    renewMacSlot();
}


int remainingMacs(const MacRange &range)
{
    auto macToInt = [](const QString &mac) -> quint64 {
        QString hex = mac;
        hex.remove(':');
        return hex.toULongLong(nullptr, 16);
    };

    quint64 start = macToInt(range.startMac);
    quint64 end = macToInt(range.endMac);
    quint64 current = macToInt(range.currentMac);

    if (current < start) current = start;
    if (current > end) return 0;
    return static_cast<int>(end - current);
}

void Setup_MainWid::renewMacSlot()
{
    const MacRange& ethRange = gen->getMacs()["mac"];
    const MacRange& zbRange  = gen->getMacs()["zigbee"];

    ui->startMacLab->setText(ethRange.startMac);
    ui->endMacLab->setText(ethRange.endMac);
    ui->CurMacLab->setText(ethRange.currentMac);
    ui->cntMacLab->setText(QString::number(remainingMacs(ethRange)));

    ui->ZBstartMacLab->setText(zbRange.startMac);
    ui->ZBendMacLab->setText(zbRange.endMac);
    ui->ZBCurMacLab->setText(zbRange.currentMac);
    ui->ZBcntMacLab->setText(QString::number(remainingMacs(zbRange)));
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
