#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QWidgetAction>
#include <QMessageBox>
#include <QDebug>

#include "yaudtrayapp.h"
#include "devinfowidget.h"

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudTrayApp::YaudTrayApp(int &argc, char **argv)
    : QApplication(argc, argv),
      trayIcon(NULL),
      trayMenu(NULL),
      aboutAction(NULL),
      exitAction(NULL)
{
    createTrayIcon();

    if (!connectToUdisks())
        exit(1);

    if (!getDevicesList())
        exit(1);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudTrayApp::~YaudTrayApp()
{
    delete aboutAction;
    aboutAction = NULL;

    delete exitAction;
    exitAction = NULL;

    delete trayMenu;
    trayMenu = NULL;

    delete trayIcon;
    trayIcon = NULL;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::createTrayIcon()
{
    // QIcon::setThemeName("Faenza-Dark");

    printf("themeName: %s\n", qPrintable(QIcon::themeName()));
    QStringList tsPaths = QIcon::themeSearchPaths();
    for (int i=0; i<tsPaths.size(); ++i)
        printf("themeSearchPaths[%d]: %s\n", i, qPrintable(tsPaths.at(i)));

    setWindowIcon(QIcon::fromTheme("media-eject"));
    setQuitOnLastWindowClosed(false);

    trayMenu = new QMenu(tr("Yaud tray"));
    trayMenu->addSeparator();
    aboutAction = trayMenu->addAction(QIcon::fromTheme("help-about"), tr("&About"), this, SLOT(onAbout()));
    exitAction = trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this, SLOT(quit()));

    trayIcon = new QSystemTrayIcon(QIcon::fromTheme("media-eject"));
    // trayIcon->setContextMenu(trayMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(showTrayMenu()));

    trayIcon->show();
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
static bool sortByUdisksPath(const QDBusObjectPath &p1, const QDBusObjectPath &p2)
{
    return p1.path() < p2.path();
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

    std::vector<QDBusObjectPath> sortVector;

    enumArg.beginArray();
    while (!enumArg.atEnd())
    {
        QDBusObjectPath devObj = qdbus_cast<QDBusObjectPath>(enumArg);
        sortVector.push_back(devObj);
    }
    enumArg.endArray();

    std::sort(sortVector.begin(), sortVector.end(), sortByUdisksPath);

    for (size_t i=0; i<sortVector.size(); ++i)
        addDevice(sortVector[i], true);

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

    DevInfoWidget *widg = new DevInfoWidget(&yaudDI, trayMenu);
    connect(widg, SIGNAL(requestProcessing(QString)), this, SLOT(processingRequested(QString)));

    QWidgetAction *widgAct = new QWidgetAction(trayMenu);
    widgAct->setDefaultWidget(widg);

    yaudDI.menuAction = widgAct;
    yaudDI.menuWidget = widg;
    devices[yaudDI.udisksPath] = yaudDI;

    trayMenu->insertAction(trayMenu->actions().at(0), widgAct);

    if (!onStart)
        showTrayMenu();
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceAdded(QDBusObjectPath device)
{
    addDevice(device);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceRemoved(QDBusObjectPath device)
{
    std::map<QString, YaudDeviceInfo>::iterator it = devices.find(device.path());
    if (it == devices.end())
        return;

    printf("[YaudTrayApp::onDeviceRemoved] : %s\n", qPrintable(device.path()));

    trayMenu->removeAction(it->second.menuAction);
    delete it->second.menuAction;
    devices.erase(it);

    if (!trayMenu->isHidden())
        showTrayMenu();
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

    it->second.convert(device);
    it->second.refreshWidget();

    if (!trayMenu->isHidden())
        showTrayMenu();
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onAbout()
{
    QMessageBox::about(NULL,
                       tr("YAUDTray"),
                       tr("<H3 ALIGN=CENTER>Yet Another Udisks Tray Mounter</H3>\n"
                          "<A HREF=\"https://github.com/salieff/yaudtray\" ALIGN=CENTER>https://github.com/salieff/yaudtray</A>"));
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::showTrayMenu()
{
    trayMenu->adjustSize();
    QPoint p(trayIcon->geometry().topLeft().x() - (trayMenu->sizeHint().width()/2), trayIcon->geometry().topLeft().y() - trayMenu->sizeHint().height());
    trayMenu->popup(p);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::processingRequested(QString udisksPath)
{
    std::map<QString, YaudDeviceInfo>::iterator it = devices.find(udisksPath);
    if (it == devices.end())
        return;

    if (it->second.isMounted)
    {
        it->second.unmount();
        if (it->second.isEjectable)
            it->second.eject();
    }
    else
    {
        it->second.mount();
    }
}
