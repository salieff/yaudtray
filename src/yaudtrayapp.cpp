#include <stdio.h>
#include <stdlib.h>

#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDebug>

#include "yaudtrayapp.h"

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudTrayApp::YaudTrayApp(int &argc, char **argv)
    : QApplication(argc, argv),
      trayIcon(NULL),
      trayMenu(NULL),
      aboutAction(NULL),
      exitAction(NULL)
{
    // QIcon::setThemeName("Faenza-Dark");

    printf("themeName: %s\n", qPrintable(QIcon::themeName()));
    QStringList tsPaths = QIcon::themeSearchPaths();
    for (int i=0; i<tsPaths.size(); ++i)
        printf("themeSearchPaths[%d]: %s\n", i, qPrintable(tsPaths.at(i)));

    trayMenu = new QMenu(tr("Yaud tray"));
    trayMenu->addSeparator();
    aboutAction = trayMenu->addAction(QIcon::fromTheme("help-about"), tr("&About"), this, SLOT(onAbout()));
    exitAction = trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this, SLOT(quit()));

    trayIcon = new QSystemTrayIcon(QIcon::fromTheme("media-eject"));
    trayIcon->setContextMenu(trayMenu);

    if (!connectToUdisks())
        exit(1);

    if (!getDevicesList())
        exit(1);

    trayIcon->show();
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudTrayApp::~YaudTrayApp()
{
    delete trayIcon;
    trayIcon = NULL;

    delete trayMenu;
    trayMenu = NULL;

//    delete aboutAction;
//    aboutAction = NULL;

//    delete exitAction;
//    exitAction = NULL;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
bool YaudTrayApp::connectToUdisks()
{
    if (!QDBusConnection::systemBus().isConnected())
    {
        fprintf(stderr, "ERROR: Can't connect to dbus daemon\n");
        return false;
    }

    if (!QDBusConnection::systemBus().connect("org.freedesktop.UDisks",
            "/org/freedesktop/UDisks",
            "org.freedesktop.UDisks",
            "DeviceAdded",
            this,
            SLOT(onDeviceAdded(QDBusObjectPath))))
    {
        fprintf(stderr, "ERROR: Can't connect to org.freedesktop.UDisks/DeviceAdded\n");
        return false;
    }


    if (!QDBusConnection::systemBus().connect("org.freedesktop.UDisks",
            "/org/freedesktop/UDisks",
            "org.freedesktop.UDisks",
            "DeviceRemoved",
            this,
            SLOT(onDeviceRemoved(QDBusObjectPath))))
    {
        fprintf(stderr, "ERROR: Can't connect to org.freedesktop.UDisks/DeviceRemoved\n");
        return false;
    }

    if (!QDBusConnection::systemBus().connect("org.freedesktop.UDisks",
            "/org/freedesktop/UDisks",
            "org.freedesktop.UDisks",
            "DeviceChanged",
            this,
            SLOT(onDeviceChanged(QDBusObjectPath))))
    {
        fprintf(stderr, "ERROR: Can't connect to org.freedesktop.UDisks/DeviceChanged\n");
        return false;
    }

    return true;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
bool YaudTrayApp::getDevicesList()
{
    QDBusInterface devEnum("org.freedesktop.UDisks",
                           "/org/freedesktop/UDisks",
                           "org.freedesktop.UDisks",
                           QDBusConnection::systemBus());

    QDBusMessage enumRes = devEnum.call("EnumerateDevices");
    if (enumRes.type() == QDBusMessage::ErrorMessage)
    {
        fprintf(stderr, "ERROR: Can't call EnumerateDevices\n");
        fprintf(stderr, "       %s : %s\n", qPrintable(enumRes.errorName()), qPrintable(enumRes.errorMessage()));
        return false;
    }

    if (enumRes.type() != QDBusMessage::ReplyMessage || !enumRes.arguments().at(0).canConvert<QDBusArgument>())
    {
        fprintf(stderr, "ERROR: Unexpected result type of EnumerateDevices call\n");
        return false;
    }

    const QDBusArgument enumArg = enumRes.arguments().at(0).value<QDBusArgument>();
    if (enumArg.currentType() != QDBusArgument::ArrayType)
    {
        fprintf(stderr, "ERROR: Unexpected argument type of EnumerateDevices call\n");
        return false;
    }

    // qDebug() << enumRes;

    enumArg.beginArray();
    while (!enumArg.atEnd())
    {
        QDBusObjectPath devObj = qdbus_cast<QDBusObjectPath>(enumArg);
        addDevice(devObj, true);
    }
    enumArg.endArray();

    return true;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::addDevice(QDBusObjectPath device, bool onStart)
{
    YaudDeviceInfo yaudDI;
    yaudDI.convert(device);
    if (!yaudDI.isExternalMountPoint)
        return;

    printf("[YaudTrayApp::onDeviceAdded]\n");
    yaudDI.print();

    devices[yaudDI.udisksPath] = yaudDI;

    trayMenu->insertAction(trayMenu->actions().at(0), new QAction(QIcon::fromTheme("drive-removable-media-usb"), yaudDI.displayName, this));
    trayMenu->adjustSize();

    if (!onStart)
    {
        QPoint p(trayIcon->geometry().topLeft().x() - (trayMenu->sizeHint().width()/2), trayIcon->geometry().topLeft().y() - trayMenu->height());
        //QPoint p(trayIcon->geometry().topLeft().x() - (trayMenu->sizeHint().width()/2), trayIcon->geometry().topLeft().y());
        trayMenu->popup(p);
    }
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceAdded(QDBusObjectPath device)
{
    addDevice(device);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceRemoved(QDBusObjectPath device)
{
    YaudDeviceInfo yaudDI;
    yaudDI.convert(device);

    printf("[YaudTrayApp::onDeviceRemoved]\n");
    yaudDI.print();

    devices.erase(yaudDI.udisksPath);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceChanged(QDBusObjectPath device)
{
    YaudDeviceInfo yaudDI;
    yaudDI.convert(device);

    if (!yaudDI.isExternalMountPoint)
    {
        onDeviceRemoved(device);
        return;
    }

    std::map<QString, YaudDeviceInfo>::iterator it = devices.find(yaudDI.udisksPath);
    if (it == devices.end())
    {
        onDeviceAdded(device);
        return;
    }

    if (it->second == yaudDI)
        return;

    printf("[YaudTrayApp::onDeviceChanged]\n");
    yaudDI.print();

    devices[yaudDI.udisksPath] = yaudDI;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onAbout()
{
}
