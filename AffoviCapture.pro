QT += widgets gui
TEMPLATE = app
TARGET = AffoviCapture
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lvlc -lvlccore
unix:LIBS += -lX11
unix:INCLUDEPATH+="/usr/include/vlc/plugins"
win32:INCLUDEPATH*="C:/Program Files (x86)/VideoLAN/VLC/sdk/include"
win32:LIBPATH+="C:/Program Files (x86)/VideoLAN/VLC/sdk/lib"

# Input
HEADERS += player.h \
    qt_dirs.h
SOURCES += main.cpp player.cpp
