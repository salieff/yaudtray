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
#include "helpers.h"

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudTrayApp::YaudTrayApp(int &argc, char **argv)
    : QApplication(argc, argv),
      trayIcon(NULL),
      mountMenu(NULL),
      aboutMenu(NULL),
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
    std::map<QString, YaudDeviceInfo*>::iterator it;
    for (it = devices.begin(); it != devices.end(); ++it)
    {
        printf("[YaudTrayApp::~YaudTrayApp] Delete %s\n", qPrintable(it->second->displayName));
        mountMenu->removeAction(it->second->menuAction);
        delete it->second->menuWidget;
        delete it->second->menuAction;
        delete it->second;
    }
    devices.clear();

    delete noMediaAction;
    delete mountMenu;

    delete aboutAction;
    delete exitAction;
    delete aboutMenu;

    delete trayIcon;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::createTrayIcon()
{
    // QIcon::setThemeName("Faenza-Dark");

    printf("themeName: %s\n", qPrintable(QIcon::themeName()));
    QStringList tsPaths = QIcon::themeSearchPaths();
    for (int i=0; i<tsPaths.size(); ++i)
        printf("themeSearchPaths[%d]: %s\n", i, qPrintable(tsPaths.at(i)));

    setWindowIcon(yaudIcon("media-eject"));
    setQuitOnLastWindowClosed(false);

    mountMenu = new QMenu(tr("Yaud tray"));
    noMediaAction = mountMenu->addAction(yaudIcon("media-playback-stop"), tr("No media available"));

    aboutMenu = new QMenu(tr("Yaud tray"));
    aboutAction = aboutMenu->addAction(yaudIcon("help-about"), tr("&About"), this, SLOT(onAbout()));
    exitAction = aboutMenu->addAction(yaudIcon("application-exit"), tr("&Quit"), this, SLOT(quit()));

    trayIcon = new QSystemTrayIcon(yaudIcon("media-eject"));
    // trayIcon->setContextMenu(aboutMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(showTrayMenu(QSystemTrayIcon::ActivationReason)));

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
    YaudDeviceInfo *yaudDI = new YaudDeviceInfo(device);
    if (!yaudDI->isExternalMountPoint)
    {
        delete yaudDI;
        return;
    }

    printf("[YaudTrayApp::onDeviceAdded]\n");
    yaudDI->print();

    //DevInfoWidget *widg = new DevInfoWidget(yaudDI);
    DevInfoWidget *widg = new DevInfoWidget(yaudDI, mountMenu);
    connect(widg, SIGNAL(requestProcessing(QString)), this, SLOT(processingRequested(QString)));
    connect(widg, SIGNAL(requestCloseError(QString)), this, SLOT(errCloseRequested(QString)));

    QWidgetAction *widgAct = new QWidgetAction(mountMenu);
    widgAct->setDefaultWidget(widg);
    //widg->show();

    yaudDI->menuAction = widgAct;
    yaudDI->menuWidget = widg;
    devices[yaudDI->udisksPath] = yaudDI;

    connect(yaudDI, SIGNAL(commandDone(QString)), this, SLOT(deviceCommandDone(QString)));
    connect(yaudDI, SIGNAL(commandError(QString)), this, SLOT(deviceCommandError(QString)));

    noMediaAction->setVisible(false);
    mountMenu->insertAction(mountMenu->actions().at(0), widgAct);

    if (!onStart)
        showTrayMenu(QSystemTrayIcon::Trigger);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceAdded(QDBusObjectPath device)
{
    addDevice(device);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceRemoved(QDBusObjectPath device)
{
    std::map<QString, YaudDeviceInfo*>::iterator it = devices.find(device.path());
    if (it == devices.end())
        return;

    printf("[YaudTrayApp::onDeviceRemoved] : %s\n", qPrintable(device.path()));

    mountMenu->removeAction(it->second->menuAction);
    delete it->second->menuWidget;
    delete it->second->menuAction;
    delete it->second;
    devices.erase(it);

    if (devices.empty())
        noMediaAction->setVisible(true);

    if (!mountMenu->isHidden())
        showTrayMenu(QSystemTrayIcon::Trigger);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceChanged(QDBusObjectPath device)
{
    std::map<QString, YaudDeviceInfo*>::iterator it = devices.find(device.path());
    if (it == devices.end())
    {
        onDeviceAdded(device);
        return;
    }

    it->second->convert(device);
    if (!it->second->isExternalMountPoint)
    {
        onDeviceRemoved(device);
        return;
    }

    printf("[YaudTrayApp::onDeviceChanged]\n");
    it->second->print();

    it->second->refreshWidget();

    if (!mountMenu->isHidden())
        showTrayMenu(QSystemTrayIcon::Trigger);
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
void YaudTrayApp::showTrayMenu(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Context :
    {
        QPoint p(trayIcon->geometry().topLeft().x() - (aboutMenu->sizeHint().width()/2), trayIcon->geometry().topLeft().y() - aboutMenu->sizeHint().height());
        aboutMenu->popup(p);
    }
    break;

    default :
    {
        if (!mountMenu->isEmpty())
        {
            mountMenu->resize(1, 1); // Hack to adjust menu size forcely
            // mountMenu->adjustSize();
            QPoint p(trayIcon->geometry().topLeft().x() - (mountMenu->sizeHint().width()/2), trayIcon->geometry().topLeft().y() - mountMenu->sizeHint().height());
            mountMenu->popup(p);
        }
        else
        {
            mountMenu->hide();
        }
    }
    break;

    }
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::processingRequested(QString udisksPath)
{
    std::map<QString, YaudDeviceInfo*>::iterator it = devices.find(udisksPath);
    if (it == devices.end())
        return;

    if (it->second->isMounted)
    {
        if (!it->second->unmount())
        {
            fprintf(stderr, "ERROR: Can't send umount command to dbus!\n");
            return;
        }

        if (!it->second->isEjectable)
            return;

        if (!it->second->eject())
        {
            fprintf(stderr, "ERROR: Can't send eject command to dbus!\n");
            return;
        }
    }
    else
    {
        if (!it->second->mount())
        {
            fprintf(stderr, "ERROR: Can't send mount command to dbus!\n");
            return;
        }
    }

    printf("Command sent OK\n");
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::deviceCommandDone(QString udisksPath)
{
    if (!mountMenu->isHidden())
        showTrayMenu(QSystemTrayIcon::Trigger);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::deviceCommandError(QString udisksPath)
{
    showTrayMenu(QSystemTrayIcon::Trigger);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::errCloseRequested(QString udisksPath)
{
    std::map<QString, YaudDeviceInfo*>::iterator it = devices.find(udisksPath);
    if (it == devices.end())
        return;

    it->second->lastError.clear();
    it->second->lastErrDescription.clear();

    it->second->refreshWidget();
    if (!mountMenu->isHidden())
        showTrayMenu(QSystemTrayIcon::Trigger);
}
