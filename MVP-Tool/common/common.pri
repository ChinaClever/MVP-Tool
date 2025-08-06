INCLUDEPATH += $$PWD

include(backcolour/backcolour.pri)
include(serialport/serialport.pri)
include(globals/globals.pri)
include(datapacket/datapacket.pri)
include(dbcom/dbcom.pri)

HEADERS += \
    $$PWD/globals/globals.h

SOURCES += \
    $$PWD/globals/globals.cpp
