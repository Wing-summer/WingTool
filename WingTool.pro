QT += core gui dtkwidget

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WingTool
TEMPLATE = app

QT += x11extras
LIBS += -lX11 -lXext -lXtst

SOURCES += \
        main.cpp \
    class/eventmonitor.cpp \
    QHotkey/qhotkey.cpp \
    QHotkey/qhotkey_x11.cpp \
    class/wingapplication.cpp \
    class/appmanager.cpp \
    dialog/centerwindow.cpp \
    dialog/toolwindow.cpp \
    dialog/toolboxwindow.cpp \
    class/settings.cpp \
    plugin/pluginsystem.cpp \
    class/settingmanager.cpp \
    dialog/shortcuteditdialog.cpp \
    control/pluginselector.cpp \
    dialog/pluginseldialog.cpp
RESOURCES +=         resources.qrc

HEADERS += \
    class/eventmonitor.h \
    QHotkey/qhotkey.h \
    QHotkey/qhotkey_p.h \
    class/wingapplication.h \
    class/appmanager.h \
    dialog/centerwindow.h \
    dialog/toolwindow.h \
    dialog/toolboxwindow.h \
    class/settings.h \
    plugin/pluginsystem.h \
    plugin/iwingtoolplg.h \
    class/settingmanager.h \
    dialog/shortcuteditdialog.h \
    control/pluginselector.h \
    dialog/pluginseldialog.h \
    utilities.h
