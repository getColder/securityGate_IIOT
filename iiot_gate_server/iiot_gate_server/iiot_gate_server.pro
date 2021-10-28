QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
QT += serialport
QT += network

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += /home/jiff/桌面/securityGate_IIOT/sctoInterface/sctoInterFace/

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        checkthread.cpp \
        iiot_g_server.cpp \
        main.cpp \
        patterns.cpp \
        qtcpserver_rimpl.cpp \
        serialthread.cpp \
        serverthread.cpp

LIBS += -L/opt/libScto -lsctoInterFace
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    checkthread.h \
    iiot_g_server.h \
    patterns.h \
    qtcpserver_rimpl.h \
    serialthread.h \
    serverthread.h
