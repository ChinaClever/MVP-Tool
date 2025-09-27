#include <QString>
#include <QRegularExpression>
#include <map>

QString comPort = "COM3";
bool allTestState = false;

QRegularExpression ifaceRegex(R"((eth0|eth1|eth2|spe0|spe1)\s+(\S+))");
QRegularExpression serialRegex(R"(SerialNumber=(\S+))"); // SerialNumber=xxx
QRegularExpression hwRegex(R"(HwRevision=(\S+))");       // HwRevision=xxx
QRegularExpression fwRegex(R"(FwRevision=(\S+))");       // FwRevision=xxx
QRegularExpression macRegex(R"(MacAddress=(\S+))");       // MacAddress=xxx
QRegularExpression zbRegex(R"(ZB\s*[:：]\s*(\S+))");           // 匹配 "ZB: xxx"（若输出含ZigBee信息）
QRegularExpression btRegex(R"(BT\s*[:：]\s*(\S+))");           // 匹配 "BT: xxx"（若输出含蓝牙信息）
QRegularExpression boardRegex(R"(BOARD_SERIAL\s*:\s*(\S+))");
