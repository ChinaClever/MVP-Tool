#ifndef DEVICEIDGENERATOR_H
#define DEVICEIDGENERATOR_H
#include <QString>
#include <QMap>
struct MacRange
{
    QString startMac;  // 起始 MAC
    QString endMac;    // 终止 MAC
    QString currentMac; // 当前已分配的 MAC
};

class DeviceIdGenerator
{
public:
    static DeviceIdGenerator& instance();
    QString getSN(const QString& type = "Smart");
    QString getMac(const QString& type);
    void initMac();
    void setMacRange(const QString& type, const QString& start, const QString& end);

    QMap<QString,MacRange>getMacs()const;

private:
    DeviceIdGenerator();
    ~DeviceIdGenerator();
    DeviceIdGenerator& operator=(const DeviceIdGenerator&) = delete;
    DeviceIdGenerator(const DeviceIdGenerator&) = delete;

    QString m_sn;
    QMap<QString,MacRange>m_macRanges; //ZB ETH

    QString normalizeMac(const QString& mac) const; // 转换成带冒号的格式
    QString incrementMac(const QString& mac);

};

#endif // DEVICEIDGENERATOR_H
