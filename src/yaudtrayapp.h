#ifndef _YAUD_TRAY_APP_H_
#define _YAUD_TRAY_APP_H_

#include <QApplication>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QSystemTrayIcon>
#include <QMenu>

#include <map>

#include "devinfo.h"

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
    void showTrayMenu(QSystemTrayIcon::ActivationReason reason);
    void processingRequested(QString udisksPath);
    void deviceCommandDone(QString udisksPath);
    void deviceCommandError(QString udisksPath);
    void errCloseRequested(QString udisksPath);

private:
    void createTrayIcon();
    bool connectToUdisks();
    bool getDevicesList();

    void addDevice(QDBusObjectPath device, bool onStart=false);

    std::map<QString, YaudDeviceInfo*> devices;

    QSystemTrayIcon *trayIcon;
    QMenu *mountMenu;
    QMenu *aboutMenu;

    QAction *aboutAction;
    QAction *exitAction;
    QAction *noMediaAction;
};

#endif /* _YAUD_TRAY_APP_H_ */
