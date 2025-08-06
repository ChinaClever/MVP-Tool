#include "navbarwid.h"
#include "ui_navbarwid.h"
#include "backcolourcom.h"

NavBarWid::NavBarWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NavBarWid)
{
    ui->setupUi(this);
    QGridLayout *gridLayout = new QGridLayout(parent);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(this);
    set_background_icon(this,":/image/title_back.jpg");
}

NavBarWid::~NavBarWid()
{
    delete ui;
}

void NavBarWid::on_homeBtn_clicked()
{
    emit navBarSig(0);
}

void NavBarWid::on_setBtn_clicked()
{
    emit navBarSig(1);
}

