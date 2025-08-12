#include "deviceidgenerator.h"
#include <QDateTime>
#include <QByteArray>
DeviceIdGenerator& DeviceIdGenerator::instance()
{
    static DeviceIdGenerator instance;
    return instance;
}

DeviceIdGenerator::DeviceIdGenerator()
{

}

DeviceIdGenerator::~DeviceIdGenerator()
{

}

void DeviceIdGenerator::setMacRange(const QString& type, const QString& start, const QString& end)
{
    MacRange range;
    range.startMac = normalizeMac(start);
    range.endMac = normalizeMac(end);
    range.currentMac.clear();
    m_macRanges[type] = range;
}

QString DeviceIdGenerator::normalizeMac(const QString& mac) const
{
    QString ret = mac.toUpper();
    ret.remove(':');   // 去掉冒号（如果有）
    ret.remove(' ');   // 去掉空格（如果有）
    return ret;
}

QString DeviceIdGenerator::getSN(const QString &type)
{
    auto secondsSinceMidnight = []() -> int {
        QTime t = QTime::currentTime();
        return t.hour() * 3600 + t.minute() * 60 + t.second();
    };

    auto dayOfYear = []() -> int {
        return QDate::currentDate().dayOfYear();
    };

    QString factor = "048"; //huizhou
    QString tp = (type == "Smart") ? "0" : ((type == "Basic") ? "1" : "2");
    QString bench = "01";
    QString YY = QDateTime::currentDateTime().toString("yy");
    QString DDD = QString::number(dayOfYear()).rightJustified(3, '0');
    QString free = "99";
    QString second = QString::number(secondsSinceMidnight()).rightJustified(5, '0');
    m_sn = factor + tp + bench + YY + DDD + free + second;

    return m_sn;
}

QString DeviceIdGenerator::getMac(const QString& type)
{
    if (!m_macRanges.contains(type))
        return QString();  // 类型不存在，返回空

    MacRange& range = m_macRanges[type];
    QString key = QString("device/%1_last_mac").arg(type.toLower());

    // 下面配置文件读写暂时注释，后续启用时取消注释
    // QSettings settings("config.ini", QSettings::IniFormat);
    // if (range.currentMac.isEmpty())
    //     range.currentMac = settings.value(key).toString();

    if (range.currentMac.isEmpty())
        range.currentMac = range.startMac;
    else
    {
        QString nextMac = incrementMac(range.currentMac);
        if (nextMac.compare(range.endMac, Qt::CaseInsensitive) > 0)
            nextMac = range.startMac;
        range.currentMac = nextMac;
    }

    // 设置当前MAC到配置文件，暂时注释
    // settings.setValue(key, range.currentMac);

    return range.currentMac;
}

QString DeviceIdGenerator::incrementMac(const QString& mac)
{
    QByteArray bytes = QByteArray::fromHex(mac.toUtf8());
    if (bytes.isEmpty()) return QString();

    for (int i = bytes.size() - 1; i >= 0; --i) {
        quint8 val = static_cast<quint8>(bytes[i]);
        val += 1;
        bytes[i] = val;
        if (val != 0)
            break;
    }

    return QString(bytes.toHex().toUpper());
}

