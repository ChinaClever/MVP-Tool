#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QProcess>
#include <QMessageBox>

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

private slots:
    void onStartTestClicked();
    void onTestItemChanged(int index);
    void onclearLogBtnclicked();


    void on_Yes_Btn_clicked();

    void on_No_Btn_clicked();

private:
    Ui::MainWindow *ui;
    QMap<QString, QString> m_testArgMap; //
    bool validateComPort(const QString& comPort);
    QProcess *process;

};
#endif // MAINWINDOW_H
