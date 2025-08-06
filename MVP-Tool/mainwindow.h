#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "navbarwid.h"
#include "MainPage/home_workwid.h"
#include "setups/setup_mainwid.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void initWid();

protected slots:
    void navBarSlot(int);

private:
    Ui::MainWindow *ui;

    NavBarWid *mNavBarWid;
    Home_WorkWid *mHomeWid;
    Setup_MainWid *mSetupWid;
};
#endif // MAINWINDOW_H
