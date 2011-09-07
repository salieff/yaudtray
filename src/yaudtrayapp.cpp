#include <stdio.h>
#include <stdlib.h>

#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusInterface>
#include <QStringList>
#include <QDebug>

#include "yaudtrayapp.h"

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudTrayApp::YaudTrayApp(int &argc, char **argv)
    : QApplication(argc, argv)
{
    if (!connectToUdisks())
        exit(1);

    if (!getDevicesList())
        exit(1);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudTrayApp::~YaudTrayApp()
{
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
    QDBusMessage enumCall = QDBusMessage::createMethodCall("org.freedesktop.UDisks",
                            "/org/freedesktop/UDisks",
                            "org.freedesktop.UDisks",
                            "EnumerateDevices");

    QDBusMessage enumRes = QDBusConnection::systemBus().call(enumCall);
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
        onDeviceAdded(devObj);
    }
    enumArg.endArray();

    return true;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceAdded(QDBusObjectPath device)
{
    QDBusInterface devIface("org.freedesktop.UDisks",
                            device.path(),
                            "org.freedesktop.UDisks.Device",
                            QDBusConnection::systemBus());

    QString devFile = devIface.property("DeviceFile").toString();
    bool devIsSystemInternal = devIface.property("DeviceIsSystemInternal").toBool();
    bool devRemovable = devIface.property("DeviceIsRemovable").toBool();
    bool devMediaAvailable = devIface.property("DeviceIsMediaAvailable").toBool();
    bool devIsDrive = devIface.property("DeviceIsDrive").toBool();
    bool devIsOpticalDisk = devIface.property("DeviceIsOpticalDisc").toBool();
    bool devIsMounted = devIface.property("DeviceIsMounted").toBool();
    qulonglong devSize = devIface.property("DeviceSize").toULongLong();
    QString devIdUsage = devIface.property("IdUsage").toString();
    QString devIdType = devIface.property("IdType").toString();
    QString devIdUuid = devIface.property("IdUuid").toString();
    QString devIdLabel = devIface.property("IdLabel").toString();
    bool devIsMediaEjectable = false;
    bool devCanDetach = false;

    if (devIsDrive)
    {
        devIsMediaEjectable = devIface.property("DriveIsMediaEjectable").toBool();
        devCanDetach = devIface.property("DriveCanDetach").toBool();
    }

    QString devMountPath="";
    if (devIsMounted)
    {
        QStringList listMountPaths = devIface.property("DeviceMountPaths").toStringList();
        if (!listMountPaths.empty())
            devMountPath = listMountPaths.at(0);
    }

#define PRINT_STRING(arg) printf("    "#arg": %s\n", qPrintable(arg));
#define PRINT_BOOL(arg) printf("    "#arg": %s\n", arg ? "TRUE" : "FALSE");
#define PRINT_ULL(arg) printf("    "#arg": %llu\n", arg);
    printf("[YaudTrayApp::onDeviceAdded]\n");
    PRINT_STRING(device.path())
    PRINT_STRING(devFile)
    PRINT_STRING(devMountPath)
    PRINT_BOOL(devIsSystemInternal)
    PRINT_BOOL(devRemovable)
    PRINT_BOOL(devMediaAvailable)
    PRINT_BOOL(devIsDrive)
    PRINT_BOOL(devIsOpticalDisk)
    PRINT_BOOL(devIsMounted)
    PRINT_ULL(devSize)
    PRINT_STRING(devIdUsage)
    PRINT_STRING(devIdType)
    PRINT_STRING(devIdUuid)
    PRINT_STRING(devIdLabel)
    PRINT_BOOL(devIsMediaEjectable)
    PRINT_BOOL(devCanDetach)
#undef PRINT_STRING
#undef PRINT_BOOL
#undef PRINT_ULL
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceRemoved(QDBusObjectPath device)
{
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudTrayApp::onDeviceChanged(QDBusObjectPath device)
{
}
