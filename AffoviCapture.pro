QT += widgets gui multimedia
TEMPLATE = app
TARGET = AffoviCapture
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lvlc -lvlccore
linux:LIBS += -lX11
linux:INCLUDEPATH+="/usr/include/vlc/plugins"
win32:INCLUDEPATH+="C:/Program Files (x86)/VideoLAN/VLC/sdk/include" "C:/Program Files (x86)/VideoLAN/VLC/sdk/include/vlc/plugins"
win32:LIBPATH+="C:/Program Files (x86)/VideoLAN/VLC/sdk/lib"
mac:INCLUDEPATH+="/Applications/VLC.app/Contents/MacOS/include"
mac:LIBPATH+="/Applications/VLC.app/Contents/MacOS/lib"

# Input
HEADERS += player.h \
    qt_dirs.h
SOURCES += main.cpp player.cpp
