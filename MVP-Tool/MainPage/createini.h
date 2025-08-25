#ifndef CREATEINI_H
#define CREATEINI_H
#include "metadatastruct.h"
#include <QObject>
#include "baseobject.h"

class createIni
{
public:
    createIni();
    static QString toIni1(sDevInfo*);
    static QString toIni2(sDevInfo*, const QString);
    static void calculateCurrentYearWeek(QString& rf);
    static QString httpPostIni(const QString& data,const QString& host);
};

#endif // CREATEINI_H
