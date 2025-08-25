#include "createIni.h"
#include "http/JQLibrary/JQNet"
#include "http/JQLibrary/jqhttpserver.h"
#include "http/httpclient.h"
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QThread>
createIni::createIni() {}

QString createIni::toIni1(sDevInfo* data) {
    // 字段名

    QString str = "BAR,IPADDR,ETH1MAC,ETH2MAC,ETH3MAC,SPE1MAC,SPE2MAC,HW,FW,SN,BTMAC,ZBMAC,DATE,QR";
    QString str2;

    auto cleanMac = [](QString mac) {
        return mac.replace(":", "");
    };

#if DeBugMode
    data->btMac = "4E6B2A9C1D7F";
    data->zbMac = "A1B2C3D4E5F6";
    data->sn = "048299999999900003";
#endif

    // 拼接字段，注意按顺序对应 str 里的字段名
    QString ss = data->sn.right(5);
    str2 += data->sn + ",";
    str2 += "https://podmaster-" + ss + ".local" + ",";
    str2 += cleanMac(data->eth1Mac) + ",";
    str2 += cleanMac(data->eth2Mac) + ",";
    str2 += cleanMac(data->eth3Mac) + ",";
    str2 += cleanMac(data->spe1Mac) + ",";
    str2 += cleanMac(data->spe2Mac) + ",";
    str2 += data->hwVersion + ",";
    str2 += data->fwVersion + ",";
    str2 += data->sn + ",";
    str2 += cleanMac(data->btMac) + ",";
    str2 += cleanMac(data->zbMac) + ",";
    //str2 += data->date + ",";
    QString rf, time, SN, BT;
    SN = data->sn;
    BT = data->btMac.replace(":", ""); // 确保蓝牙地址也去掉冒号
    calculateCurrentYearWeek(time);
    rf =  time ;
    QString qr = "https://podview.legrand.com/qr?s=" + SN + "&m=" + BT;

    str2 += rf + "," + qr;
    qDebug() << "Header:" << str;
    qDebug() << "Values:" << str2;
    return httpPostIni(str + "\n" + str2,"80"); // 返回 header 和 values，换行分隔
}

QString createIni::toIni2(sDevInfo* data, const QString mac)
{
    QString str = "MAC,SN,QR";
#if DeBugMode
    data->sn = "048299999999900006";
#endif
    // 拼接字段，注意按顺序对应 str 里的字段名
    QString ss = data->sn.right(5);

    QString str2 = "("+mac+"),"+ss+",";
    QString SN = data->sn , MAC = mac;
    QString qr = "https://podview.legrand.com/qr?s=" + SN + "&m=" + MAC;
    str2 += qr;

    return httpPostIni(str + "\n" + str2,"81");
}

QString createIni::httpPostIni(const QString& data, const QString& host) {
    // 构造 URL
    QString url = QString("http://%1:%2/Integration/MVP3/Execute").arg("192.168.1.16").arg(host);
    qDebug() << "URL:" << url;
    qDebug() << "Data:" << data;
    QString str = "";

    // 使用智能指针管理线程和 HttpClient
    QSharedPointer<QThread> thread(new QThread);
    QSharedPointer<AeaQt::HttpClient> http(new AeaQt::HttpClient);

    QEventLoop loop; // 用于等待异步回调
    http->clearAccessCache();
    http->clearConnectionCache();

    QByteArray json = data.toUtf8();
    http->post(url)
        .header("content-type", "plain")
        .onSuccess([&](QString result) {
            qDebug() << "result" << result;
            str = result;
            loop.quit();
        })
        .onFailed([&](QString error) {
            qDebug() << "error" << error;
            str = error;
            loop.quit();
        })
        .onTimeout([&](QNetworkReply*) {
            qDebug() << "http_post timeout";
            str = "timeout";
            loop.quit();
        })
        .timeout(5000) // 增加超时到 5 秒
        .body(json)
        .exec();

    // 将 http 对象移到线程
    http->moveToThread(thread.data());
    QObject::connect(thread.data(), &QThread::started, []() { /* 确保线程启动 */ });
    QObject::connect(thread.data(), &QThread::finished, &loop, &QEventLoop::quit);

    thread->start();
    loop.exec(); // 等待回调完成

    // 确保线程停止
    if (thread->isRunning()) {
        thread->quit();
        thread->wait(1000); // 等待最多 1 秒
    }

    return str;
}

void createIni::calculateCurrentYearWeek(QString& rf) {
    QDateTime currentUtc = QDateTime::currentDateTimeUtc();
    QDate currentDate = currentUtc.date();
    int year = currentDate.year();
    int week = currentDate.weekNumber();
    int shortYear = year % 100;
    rf = QString("%1W%2").arg(shortYear, 2, 10, QChar('0')).arg(week, 2, 10, QChar('0'));
}
