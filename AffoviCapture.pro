QT += widgets gui
TEMPLATE = app
TARGET = AffoviCapture
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lvlc
unix:LIBS += -lX11
win32:INCLUDEPATH*="C:/Program Files (x86)/VideoLAN/VLC/sdk/include"
win32:LIBPATH+="C:/Program Files (x86)/VideoLAN/VLC/sdk/lib"

# Input
HEADERS += player.h
SOURCES += main.cpp player.cpp
