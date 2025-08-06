#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "backcolourcom.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mNavBarWid = new NavBarWid(ui->barWid);

   // set_background_icon(this,":/image/box_back.jpg");

    initWid();
    connect(mNavBarWid, &NavBarWid::navBarSig, this, &MainWindow::navBarSlot);
}


void MainWindow::initWid()
{
    mHomeWid = new Home_WorkWid(ui->stackedWid);
    ui->stackedWid->addWidget(mHomeWid);

    mSetupWid = new Setup_MainWid(ui->stackedWid);
    ui->stackedWid->addWidget(mSetupWid);

}

void MainWindow::navBarSlot(int id)
{
    qDebug()<<"current page:  "<<id;
    ui->stackedWid->setCurrentIndex(id);
}


MainWindow::~MainWindow()
{
    delete ui;
}
