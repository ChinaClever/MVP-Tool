INCLUDEPATH += $$PWD

include(backcolour/backcolour.pri)
include(serialport/serialport.pri)
include(globals/globals.pri)
include(datapacket/datapacket.pri)
include(dbcom/dbcom.pri)
include(sqlcom/sqlcom.pri)
include(msgcom/msgcom.pri)
include(cfgcom/cfgcom.pri)
include(excel/excel.pri)
include(logcom/logcom.pri)
include(idgeneratorcom/idgeneratorcom.pri)

HEADERS += \
    $$PWD/globals/globals.h \
    $$PWD/common.h

SOURCES += \
    $$PWD/globals/globals.cpp \
    $$PWD/common.cpp
