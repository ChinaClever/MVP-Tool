INCLUDEPATH += $$PWD

QT       += serialport

HEADERS +=  \
    $$PWD/serialmanager.h \
    $$PWD/serialportdialog.h \
    $$PWD/serialstatuswid.h

    

SOURCES +=   \
    $$PWD/serialmanager.cpp \
    $$PWD/serialportdialog.cpp \
    $$PWD/serialstatuswid.cpp


FORMS += \
    $$PWD/serialportdialog.ui \
    $$PWD/serialstatuswid.ui

