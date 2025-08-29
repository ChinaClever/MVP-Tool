#include "deviceidgenerator.h"
#include <QDateTime>
#include <QTime>
#include <QSettings>
#include <QCoreApplication>
#include <QByteArray>

// 单例
DeviceIdGenerator& DeviceIdGenerator::instance()
{
    static DeviceIdGenerator inst;
    return inst;
}

DeviceIdGenerator::DeviceIdGenerator()
{
    initMac();
}

DeviceIdGenerator::~DeviceIdGenerator()
{
}

QMap<QString, MacRange> DeviceIdGenerator::getMacs() const
{
    return m_macRanges;
}

void DeviceIdGenerator::setMacRange(const QString& type, const QString& start, const QString& end)
{
    MacRange range;
    range.startMac   = normalizeMac(start);
    range.endMac     = normalizeMac(end);
    range.currentMac.clear();
    m_macRanges[type] = range;
}

void DeviceIdGenerator::initMac()
{
    setMacRange("mac",    "2C:26:5F:38:00:00", "2C:26:5F:38:FF:FF");
    setMacRange("zigbee", "1C:26:5F:38:00:00", "1C:26:5F:38:FF:FF");

    QSettings settings(QCoreApplication::applicationDirPath() + "/db/MVP3/cfg.ini", QSettings::IniFormat);
    settings.beginGroup("device");

    for (auto it = m_macRanges.begin(); it != m_macRanges.end(); ++it) {
        QString type = it.key().toLower();
        MacRange &range = it.value();

        QString last  = settings.value(type + "_last_mac", range.startMac).toString();
        QString start = settings.value(type + "_start_mac", range.startMac).toString();
        QString end   = settings.value(type + "_end_mac",   range.endMac).toString();

        range.currentMac = normalizeMac(last);
        range.startMac   = normalizeMac(start);
        range.endMac     = normalizeMac(end);

        if (!settings.contains(type + "_last_mac"))
            settings.setValue(type + "_last_mac", range.currentMac);
        if (!settings.contains(type + "_start_mac"))
            settings.setValue(type + "_start_mac", range.startMac);
        if (!settings.contains(type + "_end_mac"))
            settings.setValue(type + "_end_mac", range.endMac);
    }

    settings.endGroup();
}

QMap<QString, QList<QString>> DeviceIdGenerator::allocateBatch()
{
    m_allocated.clear();

    // 类型 mac：生成6个
    {
        QList<QString> macList;
        QString base = m_macRanges["mac"].currentMac;
        if (base.isEmpty()) base = m_macRanges["mac"].startMac;
        for (int i = 0; i < 6; ++i) {
            base = incrementMac(base);
            macList.append(base);
        }
        m_allocated["mac"] = macList;
    }

    // 类型 zigbee：生成1个
    {
        QList<QString> zbList;
        QString base = m_macRanges["zigbee"].currentMac;
        if (base.isEmpty()) base = m_macRanges["zigbee"].startMac;
        base = incrementMac(base);
        zbList.append(base);
        m_allocated["zigbee"] = zbList;
    }

    return m_allocated;
}

void DeviceIdGenerator::saveMacs()
{
    if (m_allocated.isEmpty()) return;

    QSettings settings(QCoreApplication::applicationDirPath() + "/db/MVP3/cfg.ini", QSettings::IniFormat);
    settings.beginGroup("device");

    for (auto it = m_allocated.begin(); it != m_allocated.end(); ++it) {
        const QString& type = it.key();
        const QList<QString>& list = it.value();
        if (list.isEmpty()) continue;

        QString last = list.last();
        last = normalizeMac(last);            // 强制格式化为 XX:XX:XX:XX:XX:XX
        m_macRanges[type].currentMac = last;  // 更新内存
        settings.setValue(type.toLower() + "_last_mac", last); // 保存为字符串
    }

    settings.endGroup();
    m_allocated.clear();
}


// 生成 SN
QString DeviceIdGenerator::CreateSN(const QString& type)
{
    auto secondsSinceMidnight = []() -> int {
        QTime t = QTime::currentTime();
        return t.hour() * 3600 + t.minute() * 60 + t.second();
    };

    auto dayOfYear = []() -> int {
        return QDate::currentDate().dayOfYear();
    };

    QString factor = "048"; // huizhou
    QString tp = (type == "Smart") ? "0" : ((type == "Basic") ? "1" : "2");
    QString bench = "01";
    QString YY = QDateTime::currentDateTime().toString("yy");
    QString DDD = QString::number(dayOfYear()).rightJustified(3, '0');
    QString free = "99";
    QString second = QString::number(secondsSinceMidnight()).rightJustified(5, '0');
    m_sn = factor + tp + bench + YY + DDD + free + second;

    return m_sn;
}

// MAC 格式化（转成 XX:XX:XX:XX:XX:XX）
QString DeviceIdGenerator::normalizeMac(const QString& mac) const
{
    QString hex = mac.toUpper();
    hex.remove(':');
    hex.remove(' ');
    QString ret;
    for (int i = 0; i < hex.size(); i += 2) {
        ret.append(hex.mid(i, 2));
        if (i < hex.size() - 2)
            ret.append(":");
    }
    return ret;
}

// 递增 MAC
QString DeviceIdGenerator::incrementMac(const QString& mac)
{
    // 移除冒号
    QString hex = mac;
    hex.remove(':');

    // 校验是否为合法16进制
    if (!hex.contains(QRegExp("^[0-9A-Fa-f]+$"))) {
       // qWarning() << "Invalid MAC:" << mac;
        return mac;
    }

    // 转字节数组
    QByteArray bytes = QByteArray::fromHex(hex.toUtf8());
    if (bytes.isEmpty()) return mac;

    // 从最低字节开始递增
    for (int i = bytes.size() - 1; i >= 0; --i) {
        quint8 val = static_cast<quint8>(bytes[i]);
        val += 1;
        bytes[i] = val;
        if (val != 0) break; // 不溢出就结束
    }

    // 转回 XX:XX:XX:XX:XX:XX 格式
    QString ret;
    QString hexStr = QString(bytes.toHex().toUpper());
    for (int i = 0; i < hexStr.size(); i += 2) {
        ret.append(hexStr.mid(i, 2));
        if (i < hexStr.size() - 2)
            ret.append(":");
    }

    return ret;
}

