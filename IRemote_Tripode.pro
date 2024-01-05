QT += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


LIBS += -L/usr/local/lib -lSDL2
LIBS += -L/usr/local/lib -lSDL2 -ldl -lpthread
LIBS += -L$$HOME/gamepad/lib
INCLUDEPATH += /usr/local/include
INCLUDEPATH += $$HOME/gamepad/include

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ijoystick.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ijoystick.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
