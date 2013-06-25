#include "player.h"
#include "qt_dirs.h"
#include <vlc/vlc.h>

#define qtu( i ) ((i).toUtf8().constData())

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

Mwindow::Mwindow() {
    vlcPlayer = NULL;

    /* Init libVLC */
    if((vlcObject = libvlc_new(0,NULL)) == NULL) {
        printf("Could not init libVLC");
        exit(1);
    }

    /* Display libVLC version */
    printf("libVLC version: %s\n",libvlc_get_version());

    /* Interface initialisation */
    initMenus();
    initComponents();
    playCamera();
}

Mwindow::~Mwindow() {
    if(vlcObject)
        libvlc_release(vlcObject);
}

void Mwindow::initMenus() {

    centralWidget = new QWidget;

    videoWidget = new QWidget;
    videoWidget->setAutoFillBackground( true );
    QPalette plt = palette();
    plt.setColor( QPalette::Window, Qt::black );
    videoWidget->setPalette( plt );

    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *Quit = new QAction("&Quit", this);

    Quit->setShortcut(QKeySequence("Ctrl+Q"));

    fileMenu->addAction(Quit);

    connect(Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Mwindow::initComponents() {

    broadcastBut = new QPushButton("Start Broadcasting");
    //QObject::connect(playBut, SIGNAL(clicked()), this, SLOT(play()));

    //TODO adding a mute button sounds nice

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addStretch();
    layout->addWidget(broadcastBut);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(videoWidget, 1);
    layout2->addLayout(layout);

    centralWidget->setLayout(layout2);
    setCentralWidget(centralWidget);
    resize( 600, 400);
}

void Mwindow::playCamera() {
    /* Stop if something is playing */
    if( vlcPlayer && libvlc_media_player_is_playing(vlcPlayer) )
        stop();

    /* New Media */
    //libvlc_media_t *vlcMedia = libvlc_media_new_path(vlcObject,qtu(fileOpen));
    //TODO take the name of the input device from some drop down or something
#ifdef WIN32
    libvlc_media_t *vlcMedia = libvlc_media_new_location(vlcObject,qtu(QString("dshow://")));
#else
    libvlc_media_t *vlcMedia = libvlc_media_new_location(vlcObject,qtu(QString("v4l2:///dev/video0")));
#endif
    if( !vlcMedia )
        return;

    vlcPlayer = libvlc_media_player_new_from_media (vlcMedia);
    libvlc_media_release(vlcMedia);

    /* Integrate the video in the interface */
#if defined(Q_OS_MAC)
    libvlc_media_player_set_nsobject(vlcPlayer, videoWidget->winId());
#elif defined(Q_OS_UNIX)
    libvlc_media_player_set_xwindow(vlcPlayer, videoWidget->winId());
#elif defined(Q_OS_WIN)
    libvlc_media_player_set_hwnd(vlcPlayer, (HWND)videoWidget->winId());
#endif

    /* And play */
    libvlc_media_player_play (vlcPlayer);
}

int Mwindow::changeVolume(int vol) { //Called if you change the volume slider

    if(vlcPlayer)
        return libvlc_audio_set_volume (vlcPlayer,vol);

    return 0;
}

void Mwindow::changePosition(int pos) { //Called if you change the position slider

    if(vlcPlayer) //It segfault if vlcPlayer don't exist
        libvlc_media_player_set_position(vlcPlayer,(float)pos/(float)1000);
}

void Mwindow::stop() {
    if(vlcPlayer) {
        libvlc_media_player_stop(vlcPlayer);
        libvlc_media_player_release(vlcPlayer);
    }
    vlcPlayer = NULL;
}

void Mwindow::closeEvent(QCloseEvent *event) {
    stop();
    event->accept();
}

void Mwindow::setMediaOptions(libvlc_media_t *p_md, const QString &options) {
    /* Take options from the UI, not from what we stored */
    QStringList optionsList =options.split( " :" );

    /* Insert options */
    for( int j = 0; j < optionsList.count(); j++ )
    {
        QString qs = colon_unescape( optionsList[j] );
        if( !qs.isEmpty() )
        {
            libvlc_media_add_option(p_md, qtu( qs ));
#ifdef DEBUG_QT
            msg_Warn( p_intf, "Input option: %s", qtu( qs ) );
#endif
        }
    }
}
