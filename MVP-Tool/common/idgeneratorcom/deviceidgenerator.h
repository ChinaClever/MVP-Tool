#ifndef DEVICEIDGENERATOR_H
#define DEVICEIDGENERATOR_H

#include <QString>
#include <QMap>
#include <QList>

struct MacRange
{
    QString startMac;   // 起始 MAC
    QString endMac;     // 终止 MAC
    QString currentMac; // 当前已分配的 MAC
};

class DeviceIdGenerator
{
public:
    static DeviceIdGenerator& instance();

    QString CreateSN(const QString& type = "Smart");
    QString getSN(){return m_sn;}
    //  一次性分配一批 MAC（mac=6个, zigbee=1个），缓存到类里面
    QMap<QString, QList<QString>> allocateBatch();

    //  成功烧录后调用，才会写配置文件并更新 currentMac
    void saveMacs();

    // 获取当前范围配置
    QMap<QString, MacRange> getMacs() const;

    // 初始化范围
    void initMac();
    void setMacRange(const QString& type, const QString& start, const QString& end);
    int getRemainingMacCount(const QString& type) const;
    bool canAllocateMac(const QString& type, int requiredCount = 1) const;

private:
    DeviceIdGenerator();
    ~DeviceIdGenerator();
    DeviceIdGenerator(const DeviceIdGenerator&) = delete;
    DeviceIdGenerator& operator=(const DeviceIdGenerator&) = delete;

    QString incrementMac(const QString& mac);
    QString normalizeMac(const QString& mac) const;

    QString m_sn;
    QMap<QString, MacRange> m_macRanges;                // MAC 范围信息
    QMap<QString, QList<QString>> m_allocated;          //  最近生成的一批 MAC
};

#endif // DEVICEIDGENERATOR_H
