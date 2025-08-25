INCLUDEPATH += $$PWD

include(users/users.pri)
include(flash/flash.pri)

FORMS += \
    $$PWD/setup_mainwid.ui

HEADERS += \
    $$PWD/setup_mainwid.h

SOURCES += \
    $$PWD/setup_mainwid.cpp
