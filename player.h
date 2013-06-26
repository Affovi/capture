#ifndef PLAYER
#define PLAYER

#include <QtGui>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <vlc/vlc.h>

class Mwindow : public QMainWindow {

    Q_OBJECT

        public:
               Mwindow();
               virtual ~Mwindow();

        private slots:
               void stop();
               void broadcast(bool);
               void refreshPlayer();

               int changeVolume(int);
               void changePosition(int);

        protected:
               virtual void closeEvent(QCloseEvent*);

        private:
               QString current;
               QPushButton *broadcastBut;
               QWidget *videoWidget;
               QWidget *centralWidget;
               libvlc_instance_t *vlcObject;
               libvlc_media_player_t *vlcPlayer;
               bool broadcasting;

               QComboBox *videoDeviceCombo;

               void initMenus();
               void initComponents();
               QWidget *createCaptureDevicePanel();
               void playMedia(libvlc_media_t *vlcMedia);
               void setMediaOptions(libvlc_media_t *p_md, const QString &options);
};


#endif
