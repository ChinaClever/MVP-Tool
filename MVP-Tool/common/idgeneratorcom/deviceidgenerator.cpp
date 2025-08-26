#include "deviceidgenerator.h"
#include <QDateTime>
#include <QByteArray>
#include <QSettings>
#include <QCoreApplication>
#include <QMap>
DeviceIdGenerator& DeviceIdGenerator::instance()
{
    static DeviceIdGenerator instance;
    return instance;
}

DeviceIdGenerator::DeviceIdGenerator()
{
    initMac();
}

DeviceIdGenerator::~DeviceIdGenerator()
{

}

QMap<QString,MacRange>DeviceIdGenerator::getMacs()const
{
    return m_macRanges;
}

void DeviceIdGenerator::setMacRange(const QString& type, const QString& start, const QString& end)
{
    MacRange range;
    range.startMac = normalizeMac(start);
    range.endMac = normalizeMac(end);
    range.currentMac.clear();
    m_macRanges[type] = range;
}

void DeviceIdGenerator::initMac()
{
    setMacRange("mac","2C:26:5F:38:00:00","2C:26:5F:38:00:00");
    setMacRange("zigbee","1C:26:5F:38:00:00","1C:26:5F:38:00:00");

    QSettings settings(QCoreApplication::applicationDirPath() + "/db/MVP3/cfg.ini",QSettings::IniFormat);
    settings.beginGroup("device");

    for(auto it = m_macRanges.begin(); it != m_macRanges.end(); ++it){
        const QString type = it.key().toLower();
        MacRange &range = it.value();

        QString last = type + "_last_mac";
        QString start = type + "_start_mac";
        QString end = type + "_end_mac";

        range.currentMac = settings.value(last,range.startMac).toString();
        range.startMac   = settings.value(start,range.startMac).toString();
        range.endMac   = settings.value(end,range.endMac).toString();

        if(!settings.contains(last)){
            settings.setValue(last,range.currentMac);
        }
        if(!settings.contains(start)){
            settings.setValue(start,range.startMac);
        }
        if(!settings.contains(end)){
            settings.setValue(end,range.endMac);
        }
    }
    settings.endGroup();
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

    if(range.currentMac.isEmpty()){
        range.currentMac = range.startMac;
    }
    else{
        QString nextMac = incrementMac(range.currentMac);
        if(nextMac.compare(range.endMac,Qt::CaseInsensitive)>0)
                nextMac = range.startMac;
       range.currentMac = nextMac;
    }

    QSettings settings(QCoreApplication::applicationDirPath() + "/db/MVP3/cfg.ini");
    settings.beginGroup("device");
    QString typeKey = type.toLower() + "_last_mac";
    settings.setValue(typeKey,range.currentMac);
    settings.endGroup();
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

