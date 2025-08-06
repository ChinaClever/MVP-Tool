QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(common/common.pri)
include(MainPage/mainpage.pri)
include(setups/setups.pri)
include(http/http.pri)
include(sqlcom/sqlcom.pri)

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    navbarwid.cpp

HEADERS += \
    mainwindow.h \
    navbarwid.h

FORMS += \
    mainwindow.ui \
    navbarwid.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images/image.qrc

DISTFILES += \
    MVP3.py \
    images/back.jpg \
    images/box_back.jpg \
    images/logo.jpg \
    images/title_back.jpg
