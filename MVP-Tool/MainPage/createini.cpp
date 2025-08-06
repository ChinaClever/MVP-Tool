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

QString createIni::toIni1(const InterfaceInfo& data) {
    // 字段名
    QString str = "ETH1MAC,ETH2MAC,ETH3MAC,SPE1MAC,SPE2MAC,HW,FW,SN,BTMAC,ZBMAC,DATE,QR";
    QString str2;

    // 填充字段值，去掉 MAC 地址中的冒号
    for (int i = 0; i < data.v.size(); ++i) {
        QString value = data.v[i];
        // 对于 MAC 地址字段（索引 0, 1, 2, 3, 4, 8, 9 对应 eth0, eth1, eth2, spe0, spe1, blueT, zB）
        if (i == 0 || i == 1 || i == 2 || i == 3 || i == 4  || i == 8 || i == 9) {
            value = value.replace(":", "");
        }
        str2 += value + ",";
    }

    // 计算 RF 和 QR
    QString rf, time, SN, BT;
    SN = data.serialNum();
    BT = data.blueT().replace(":", ""); // 确保蓝牙地址也去掉冒号
    calculateCurrentYearWeek(time);
    rf =  time ;
    QString qr = "https://podview.legrand.com/qr?s=" + SN + "&m=" + BT;

    str2 += rf + "," + qr;
    qDebug() << "Header:" << str;
    qDebug() << "Values:" << str2;
    return httpPostIni(str + "\n" + str2,"80"); // 返回 header 和 values，换行分隔
}

QString createIni::toIni2(const InterfaceInfo& data)
{
    QString str = "MAC,SN,QR";
    QString str2 = data.eth0()+","+data.serialNum()+",";
    QString SN = data.serialNum() , MAC = data.eth0();
    QString qr = "https://podview.legrand.com/qr?s=" + SN + "&m=" + MAC;
    str2 += qr;

    return httpPostIni(str + "\n" + str2,"81");
}

QString createIni::httpPostIni(const QString& data, const QString& host) {
    // 构造 URL
    QString url = QString("http://%1:%2/Integration/MVP3/Execute").arg("127.0.0.1").arg(host);
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
