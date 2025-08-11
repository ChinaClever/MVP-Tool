/*
 *
 *  Created on: 2020年10月1日
 *      Author: Lzy
 */
#include "logmainwid.h"
#include "ui_logmainwid.h"

LogMainWid::LogMainWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogMainWid)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
    QTimer::singleShot(rand()%50,this,SLOT(initFunSLot()));
}

LogMainWid::~LogMainWid()
{
    delete ui;
}


void LogMainWid::initFunSLot()
{
    QString str = tr("结果日志");
    mLogWid = new LogComWid(ui->tabWidget);
    mLogWid->initWid(DbLogs::bulid(),new LogBtnBar(),  new Log_LogQueryDlg(this), str);
    ui->tabWidget->addTab(mLogWid, str);

    // str = tr("过程日志");
    // mStatusWid = new LogComWid(ui->tabWidget);
    // mStatusWid->initWid(DbStates::bulid(),new LogBtnBar(),  new Log_LogQueryDlg(this), str);
    // ui->tabWidget->addTab(mStatusWid, str);

    // str = tr("设备Mac记录");
    // mMacWid = new LogComWid(ui->tabWidget);
    // mMacWid->initWid(DbMacs::bulid(),new LogBtnBar(),  new Log_MacQueryDlg(this), str);
    // ui->tabWidget->addTab(mMacWid, str);
}

