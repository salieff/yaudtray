#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QWidgetAction>
#include <QMessageBox>
#include <QSizePolicy>
#include <QDebug>

#include "yaudtrayapp.h"
#include "devinfowidget.h"
#include "helpers.h"

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
NoMediaWidget::NoMediaWidget(QWidget *parent)
    : QWidget(parent),
      iconLabel(NULL),
      textLabel(NULL)
{
    hbLayout = new QHBoxLayout(this);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    iconLabel = new QLabel(this);
    iconLabel->setSizePolicy(sizePolicy);
    iconLabel->setMinimumSize(QSize(48, 48));
    iconLabel->setMaximumSize(QSize(48, 48));
    iconLabel->setPixmap(yaudIcon("media-playback-stop").pixmap(48, 48));

    hbLayout->addWidget(iconLabel);

    textLabel = new QLabel(tr("No media available"), this);
    QFont font;
    font.setBold(true);
    font.setItalic(true);
    font.setWeight(75);
    textLabel->setFont(font);

    hbLayout->addWidget(textLabel);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
NoMediaWidget::~NoMediaWidget()
{
    delete iconLabel;
    delete textLabel;
    delete hbLayout;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudTrayApp::YaudTrayApp(int &argc, char **argv)
    : QApplication(argc, argv),
      trayIcon(NULL),
      mountWidget(NULL),
      mountLayout(NULL),
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
        mountLayout->removeWidget(it->second->menuWidget);
        delete it->second->menuWidget;
        delete it->second;
    }
    devices.clear();

    mountLayout->removeWidget(noMediaWidget);
    delete noMediaWidget;
    delete mountLayout;
    delete mountWidget;

    delete aboutAction;
    delete exitAction;
    delete aboutMenu;

    delete trayIcon;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::createTrayIcon()
{
    printf("themeName: %s\n", qPrintable(QIcon::themeName()));
    QStringList tsPaths = QIcon::themeSearchPaths();
    for (int i=0; i<tsPaths.size(); ++i)
        printf("themeSearchPaths[%d]: %s\n", i, qPrintable(tsPaths.at(i)));

    setWindowIcon(yaudIcon("media-eject"));
    setQuitOnLastWindowClosed(false);

    mountWidget = new QWidget(NULL, Qt::Popup);
    mountLayout = new QVBoxLayout(mountWidget);
    noMediaWidget = new NoMediaWidget(mountWidget);
    noMediaWidget->show();
    mountLayout->addWidget(noMediaWidget);

    aboutMenu = new QMenu(tr("Yaud tray"));
    aboutAction = aboutMenu->addAction(yaudIcon("help-about"), tr("&About"), this, SLOT(onAbout()));
    exitAction = aboutMenu->addAction(yaudIcon("application-exit"), tr("&Quit"), this, SLOT(quit()));

    trayIcon = new QSystemTrayIcon(yaudIcon("media-eject"));
    trayIcon->setContextMenu(aboutMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(toggleTrayMenu(QSystemTrayIcon::ActivationReason)));

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

    DevInfoWidget *widg = new DevInfoWidget(yaudDI, mountWidget);
    connect(widg, SIGNAL(requestProcessing(QString)), this, SLOT(processingRequested(QString)));
    connect(widg, SIGNAL(requestCloseError(QString)), this, SLOT(errCloseRequested(QString)));

    widg->show();

    yaudDI->menuWidget = widg;
    devices[yaudDI->udisksPath] = yaudDI;

    connect(yaudDI, SIGNAL(commandDone(QString)), this, SLOT(deviceCommandDone(QString)));
    connect(yaudDI, SIGNAL(commandError(QString)), this, SLOT(deviceCommandError(QString)));

    noMediaWidget->hide();
    mountLayout->insertWidget(0, widg);

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
    std::map<QString, YaudDeviceInfo*>::iterator it = devices.find(device.path());
    if (it == devices.end())
        return;

    printf("[YaudTrayApp::onDeviceRemoved] : %s\n", qPrintable(device.path()));

    mountLayout->removeWidget(it->second->menuWidget);
    delete it->second->menuWidget;
    delete it->second;
    devices.erase(it);

    if (devices.empty())
        noMediaWidget->show();

    if (!mountWidget->isHidden())
        showTrayMenu();
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

    if (!mountWidget->isHidden())
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
    mountWidget->adjustSize();
    QPoint p(trayIcon->geometry().topLeft().x() - (mountWidget->width()/2), trayIcon->geometry().topLeft().y() - mountWidget->height());
    mountWidget->move(p);
    mountWidget->show();
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::toggleTrayMenu(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Context)
        return;

    if (mountWidget->isHidden())
        showTrayMenu();
    else
        mountWidget->hide();
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
            fprintf(stderr, "ERROR: Can't send umount command to dbus!\n");
    }
    else
    {
        if (!it->second->mount())
            fprintf(stderr, "ERROR: Can't send mount command to dbus!\n");
    }
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::deviceCommandDone(QString udisksPath)
{
    std::map<QString, YaudDeviceInfo*>::iterator it = devices.find(udisksPath);
    if (it == devices.end())
        return;

    // Eject CD after unmount
    if (it->second->isEjectable && !it->second->isMounted)
    {
        if (!it->second->eject())
            fprintf(stderr, "ERROR: Can't send eject command to dbus!\n");
    }

    if (!mountWidget->isHidden())
        showTrayMenu();
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::deviceCommandError(QString udisksPath)
{
    showTrayMenu();
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
    if (!mountWidget->isHidden())
        showTrayMenu();
}
