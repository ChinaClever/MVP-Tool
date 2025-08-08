/*
 *
 *  Created on: 2020年10月1日
 *      Author: Lzy
 */
#include "usermainwid.h"
#include "ui_usermainwid.h"

UserMainWid::UserMainWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserMainWid)
{
    ui->setupUi(this);

    LandingUser::get();
    mBtnBar = new UserBtnBar;
    mDbTable = new SqlTableWid(ui->groupBox);
    mDbTable->initWid(DbUser::build(), mBtnBar);
}

UserMainWid::~UserMainWid()
{
    delete ui;
}
