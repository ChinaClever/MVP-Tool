#ifndef DEVICEIDGENERATOR_H
#define DEVICEIDGENERATOR_H
#include <QString>
#include <QMap>
#include <QObject>
struct MacRange
{
    QString startMac;  // 起始 MAC
    QString endMac;    // 终止 MAC
    QString currentMac; // 当前已分配的 MAC
};

struct Macs{
    QString MAC;
    QString MAC1;
    QString MAC2;
    QString MAC3;
    QString MAC4;
    QString BLUETOOTH_MAC;
    QString ZIGBEE_MAC;
    QString BOARD_SERIAL = "2Q51234567";
    QString UNIT_SERIAL;
    QString SN;
};

class DeviceIdGenerator : public QObject
{
    Q_OBJECT
public:
    static DeviceIdGenerator& instance();
    QString CreateSN(const QString& type = "Smart");
    QString getSN(){return m_sn;}
    QString getMac(const QString& type);

    void initMac(bool x);
    void setMacRange(const QString& type, const QString& start, const QString& end);

    QMap<QString,MacRange>getMacs()const;
    void setImg(QString& img){this->img = img;}
    QString getImg(){return this->img;}
    void wirteMac(const QString &type);
    QString formatMacWithColons(const QString& mac) const;


    Macs mac;

public slots:
     void setMacs(bool );

private:
    DeviceIdGenerator();
    ~DeviceIdGenerator();
    DeviceIdGenerator& operator=(const DeviceIdGenerator&) = delete;
    DeviceIdGenerator(const DeviceIdGenerator&) = delete;

    QString img;
    QString m_sn;
    QMap<QString,MacRange>m_macRanges; //ZB ETH

    QString normalizeMac(const QString& mac) const; // 转换成带冒号的格式
    QString incrementMac(const QString& mac);

};

#endif // DEVICEIDGENERATOR_H
