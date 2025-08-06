// metadatastruct.h
#ifndef METADATASTRUCT_H
#define METADATASTRUCT_H

#include <QString>
#include <QVector>
#include <map>

struct InterfaceInfo {
    QVector<QString> v; // 仍然使用 QVector 存储所有接口信息

    // 默认构造函数
    InterfaceInfo() {
        v.resize(10); // 确保 v 有 10 个元素，与原来保持一致
    }

    // 函数：从 std::map<int, QString> 填充数据
    void setFromMap(const std::map<int, QString>& map) {
        for (int i = 0; i < 10; ++i) {
            auto it = map.find(i);
            if (it != map.end()) {
                v[i] = it->second; // 如果 map 中有该键，赋值
            } else {
                v[i] = QString(); // 否则填充空 QString
            }
        }
    }

    // Getters 保持不变
    QString eth0() const { return v.size() > 0 ? v[0] : QString(); }
    QString eth1() const { return v.size() > 1 ? v[1] : QString(); }
    QString eth2() const { return v.size() > 2 ? v[2] : QString(); }
    QString spe0() const { return v.size() > 3 ? v[3] : QString(); }
    QString spe1() const { return v.size() > 4 ? v[4] : QString(); }
    QString hwRevision() const { return v.size() > 5 ? v[5] : QString(); }
    QString fwRevision() const { return v.size() > 6 ? v[6] : QString(); }
    QString serialNum() const { return v.size() > 7 ? v[7] : QString(); }
    QString blueT() const { return v.size() > 8 ? v[8] : QString(); }
    QString zB() const { return v.size() > 9 ? v[9] : QString(); }

};

#endif // METADATASTRUCT_H
