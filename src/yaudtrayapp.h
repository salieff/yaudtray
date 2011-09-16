#ifndef _YAUD_TRAY_APP_H_
#define _YAUD_TRAY_APP_H_

#include <QApplication>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QLabel>
#include <QWidget>

#include <map>

#include "devinfo.h"

class NoMediaWidget : public QWidget {
public :
    NoMediaWidget(QWidget *parent = 0);
    virtual ~NoMediaWidget();

private :
    QLabel *iconLabel;
    QLabel *textLabel;
    QHBoxLayout *hbLayout;
};

class YaudTrayApp : public QApplication
{
    Q_OBJECT

public:
    explicit YaudTrayApp(int &argc, char **argv);
    virtual ~YaudTrayApp();

private slots:
    void onDeviceAdded(QDBusObjectPath device);
    void onDeviceRemoved(QDBusObjectPath device);
    void onDeviceChanged(QDBusObjectPath device);
    void onAbout();
    void toggleTrayMenu(QSystemTrayIcon::ActivationReason reason);
    void processingRequested(QString udisksPath);
    void deviceCommandDone(QString udisksPath);
    void deviceCommandError(QString udisksPath);
    void errCloseRequested(QString udisksPath);

private:
    void createTrayIcon();
    bool connectToUdisks();
    bool getDevicesList();
    void showTrayMenu();

    void addDevice(QDBusObjectPath device, bool onStart=false);

    std::map<QString, YaudDeviceInfo*> devices;

    QSystemTrayIcon *trayIcon;
    QWidget *mountWidget;
    QVBoxLayout *mountLayout;

    QMenu *aboutMenu;

    QAction *aboutAction;
    QAction *exitAction;
    NoMediaWidget *noMediaWidget;
};

#endif /* _YAUD_TRAY_APP_H_ */
