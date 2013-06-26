#include "player.h"
#include "qt_dirs.h"
#include <vlc/vlc.h>
#include <vlc/plugins/vlc_common.h>
#include <vlc/plugins/vlc_modules.h>

#define qtu( i ) ((i).toUtf8().constData())

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

enum {
    V4L2_DEVICE,
    PVR_DEVICE,
    DTV_DEVICE,
    DSHOW_DEVICE,
    SCREEN_DEVICE,
    JACK_DEVICE
};

Mwindow::Mwindow() : broadcasting(false) {
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
    broadcastBut->setCheckable(true);
    QObject::connect(broadcastBut, SIGNAL(toggled(bool)), this, SLOT(broadcast(bool)));

    //TODO adding a mute button sounds nice

    setCentralWidget(centralWidget);

    QHBoxLayout *centalLayout = new QHBoxLayout;
    centralWidget->setLayout(centalLayout);

    centalLayout->addWidget(videoWidget, 1);

    QVBoxLayout *optionsLayout = new QVBoxLayout;
    QWidget *captureDevicePanel = createCaptureDevicePanel();
    optionsLayout->addWidget(captureDevicePanel);
    optionsLayout->addStretch();
    optionsLayout->addWidget(broadcastBut);
    centalLayout->addLayout(optionsLayout);

    resize( 600, 400);
}

QWidget* Mwindow::createCaptureDevicePanel() {
    QWidget *panel = new QWidget(this);
    QGridLayout *layout = new QGridLayout();
    panel->setLayout(layout);

    QLabel *videoDeviceLabel = new QLabel("Video:");
    layout->addWidget(videoDeviceLabel, 0, 0);
    videoDeviceCombo = new QComboBox();
    QObject::connect(videoDeviceCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshPlayer()));
    layout->addWidget(videoDeviceCombo, 0, 1);

    layout->addWidget(new QLabel("Audio:"), 1, 0);
    audioDeviceCombo = new QComboBox();
    layout->addWidget(audioDeviceCombo, 1, 1);

#ifdef Q_OS_WIN
    /*******
     * DSHOW*
     *******/
    //TODO
#else

    /*******
     * V4L2*
     *******/
    if( module_exists( "v4l2" ) ) {
        char const * const ppsz_v4lvdevices[] = {
                "video*"
        };

        //add video devices
        QStringList videoDevicesStringList = QStringList();
        for ( size_t i = 0; i< sizeof(ppsz_v4lvdevices) / sizeof(*ppsz_v4lvdevices); i++ ) {
                videoDevicesStringList << QString( ppsz_v4lvdevices[ i ] );
        }
        QStringList foundVideoDevices = QDir( "/dev/" ).entryList( videoDevicesStringList, QDir::System ).replaceInStrings( QRegExp("^"), "/dev/" );
        for (int i = 0; i < foundVideoDevices.size(); i++) {
            videoDeviceCombo->addItem(foundVideoDevices.at(i), QVariant(V4L2_DEVICE));
        }

        //add audio devices
        QStringList patterns = QStringList();
        patterns << QString( "pcmC*D*c" );

        QStringList nodes = QDir( "/dev/snd" ).entryList( patterns, QDir::System );
        QStringList names = nodes.replaceInStrings( QRegExp("^pcmC"), "hw:" )
                                         .replaceInStrings( QRegExp("c$"), "" )
                                         .replaceInStrings( QRegExp("D"), "," );
        for (int i = 0; i<names.size(); i++) {
            audioDeviceCombo->addItem( names.at(i) );
        }
    }
#endif

    /**********
     * Screen *
     **********/
    videoDeviceCombo->addItem("Desktop", QVariant(SCREEN_DEVICE));

    return panel;
}

void Mwindow::broadcast(bool broadcastringSwitch) {
    broadcasting = broadcastringSwitch;
    broadcastBut->setText(broadcasting ? "Pause Broadcasting" : "Start Broadcasting");
    refreshPlayer();
}

void Mwindow::refreshPlayer() {
    /* New Media */
    //libvlc_media_t *vlcMedia = libvlc_media_new_path(vlcObject,qtu(fileOpen));

    if (videoDeviceCombo->currentIndex() < 0) {
        //empty combo. Propbably no capture device are connected to this system
        return;
    }

    int deviceType = videoDeviceCombo->itemData(videoDeviceCombo->currentIndex()).toInt();
    const QString deviceText = videoDeviceCombo->currentText();

    const QString audioText = audioDeviceCombo->currentText();

    QString additionalTransoceOptions = "";
    libvlc_media_t *vlcMedia = NULL;
    switch (deviceType) {
#ifdef WIN32
    case DSHOW_DEVICE:
        vlcMedia = libvlc_media_new_location(vlcObject,qtu(QString("dshow://")));
        break;
#else
    case V4L2_DEVICE:
        vlcMedia = libvlc_media_new_location(vlcObject,qtu(QString("v4l2://") + deviceText));
        setMediaOptions(vlcMedia, QString("  :input-slave=alsa://") + audioText);
        break;
#endif
    case SCREEN_DEVICE:
        vlcMedia = libvlc_media_new_location(vlcObject, qtu(QString("screen://")));
        setMediaOptions(vlcMedia, QString (" :screen-fps=8.0"));//TODO take the fps from a number input
        setMediaOptions(vlcMedia, QString("  :input-slave=alsa://") + audioText);
        additionalTransoceOptions += ",width=720"; //TODO take the fps from a number input
        break;
    default:
        QMessageBox::warning(this, "Unknown device type", "The delected device's type is not recognized");
    }

    if( !vlcMedia )
        return;

    if (broadcasting) {
        setMediaOptions(vlcMedia, " :sout=" + colon_escape("#duplicate{dst={transcode{vcodec=h264,vb=800,acodec=aac,ab=128,channels=2,samplerate=44100" + additionalTransoceOptions + "}:http{mux=ts,dst=0.0.0.0:8091/}},dst=display}"));
    }

    playMedia (vlcMedia);
    libvlc_media_release(vlcMedia);
}

void Mwindow::playMedia(libvlc_media_t *vlcMedia) {
    /* Stop if something is playing */
    if( vlcPlayer && libvlc_media_player_is_playing(vlcPlayer) )
        stop();

    if( !vlcMedia )
        return;

    vlcPlayer = libvlc_media_player_new_from_media (vlcMedia);

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
