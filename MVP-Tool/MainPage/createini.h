#ifndef CREATEINI_H
#define CREATEINI_H
#include "metadatastruct.h"
#include <QObject>

class createIni
{
public:
    createIni();
    static QString toIni1(const InterfaceInfo&);
    static QString toIni2(const InterfaceInfo&);
    static void calculateCurrentYearWeek(QString& rf);
    static QString httpPostIni(const QString& data,const QString& host);
};

#endif // CREATEINI_H
