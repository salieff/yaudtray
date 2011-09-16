#include <QDBusInterface>
#include <QStringList>

#include "devinfo.h"
#include "devinfowidget.h"

#include <stdio.h>

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudDeviceInfo::reset()
{
    udisksPath.clear();
    displayName.clear();

    isExternalMountPoint = false;
    isMounted = false;
    isEjectable = false;

    size = 0;
    fsName.clear();
    mountPath.clear();

    driveType = DRT_UNKNOWN;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudDeviceInfo::YaudDeviceInfo()
    : QObject(),
      menuAction(NULL),
      menuWidget(NULL)
{
    reset();
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudDeviceInfo::YaudDeviceInfo(QDBusObjectPath device)
    : QObject(),
      menuAction(NULL),
      menuWidget(NULL)
{
    convert(device);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudDeviceInfo::convert(QDBusObjectPath device)
{
    reset();

    QDBusInterface devIface("org.freedesktop.UDisks",
                            device.path(),
                            "org.freedesktop.UDisks.Device",
                            QDBusConnection::systemBus());

    udisksPath = device.path();

    displayName = devIface.property("IdLabel").toString();
    if (displayName.isEmpty())
        displayName = devIface.property("DeviceFile").toString();
    if (displayName.isEmpty())
        displayName = devIface.property("IdUuid").toString();

    if (!devIface.property("DeviceIsSystemInternal").toBool() &&
            devIface.property("DeviceIsMediaAvailable").toBool() &&
            devIface.property("IdUsage").toString() == "filesystem")
        isExternalMountPoint = true;

    isMounted = devIface.property("DeviceIsMounted").toBool();

    if (devIface.property("DeviceIsDrive").toBool())
        isEjectable = devIface.property("DriveIsMediaEjectable").toBool();

    size = devIface.property("DeviceSize").toULongLong();
    fsName = devIface.property("IdType").toString();

    if (isMounted)
    {
        QStringList listMountPaths = devIface.property("DeviceMountPaths").toStringList();
        if (!listMountPaths.empty())
            mountPath = listMountPaths.at(0);
    }

    if (devIface.property("DeviceIsOpticalDisc").toBool())
        driveType = DRT_CDROM;
    else
        driveType = DRT_FLASH;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudDeviceInfo::refreshWidget()
{
    if (menuWidget == NULL)
        return;

    menuWidget->convertFrom(this);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudDeviceInfo::print()
{
#define PRINT_STRING(arg) printf("    "#arg": %s\n", qPrintable(arg));
#define PRINT_BOOL(arg) printf("    "#arg": %s\n", arg ? "TRUE" : "FALSE");
#define PRINT_ULL(arg) printf("    "#arg": %llu\n", arg);
    PRINT_STRING(udisksPath)
    PRINT_STRING(displayName)
    PRINT_BOOL(isExternalMountPoint)
    PRINT_BOOL(isMounted)
    PRINT_BOOL(isEjectable)
    PRINT_ULL(size)
    PRINT_STRING(fsName)
    PRINT_STRING(mountPath)

    switch (driveType)
    {
    case DRT_UNKNOWN :
        printf("    driveType: DRT_UNKNOWN\n");
        break;

    case DRT_FLASH :
        printf("    driveType: DRT_FLASH\n");
        break;

    case DRT_CDROM :
        printf("    driveType: DRT_CDROM\n");
        break;
    }
#undef PRINT_STRING
#undef PRINT_BOOL
#undef PRINT_ULL
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
bool YaudDeviceInfo::mount()
{
    QDBusInterface devIface("org.freedesktop.UDisks",
                            udisksPath,
                            "org.freedesktop.UDisks.Device",
                            QDBusConnection::systemBus());

    QList<QVariant> args;
    args << QVariant(QString()) << QVariant(QStringList());

    return devIface.callWithCallback("FilesystemMount", args, this, SLOT(onCommandDone(QDBusMessage)), SLOT(onCommandError(QDBusError, QDBusMessage)));
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
bool YaudDeviceInfo::unmount()
{
    QDBusInterface devIface("org.freedesktop.UDisks",
                            udisksPath,
                            "org.freedesktop.UDisks.Device",
                            QDBusConnection::systemBus());

    QList<QVariant> args;
    args << QVariant(QStringList());

    return devIface.callWithCallback("FilesystemUnmount", args, this, SLOT(onCommandDone(QDBusMessage)), SLOT(onCommandError(QDBusError, QDBusMessage)));
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
bool YaudDeviceInfo::eject()
{
    QDBusInterface devIface("org.freedesktop.UDisks",
                            udisksPath,
                            "org.freedesktop.UDisks.Device",
                            QDBusConnection::systemBus());

    QList<QVariant> args;
    args << QVariant(QStringList());

    return devIface.callWithCallback("DriveEject", args, this, SLOT(onCommandDone(QDBusMessage)), SLOT(onCommandError(QDBusError, QDBusMessage)));
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudDeviceInfo::onCommandDone(QDBusMessage msg)
{
    printf("[YaudDeviceInfo::onCommandDone]\n");
    lastError.clear();
    lastErrDescription.clear();
    refreshWidget();
    emit commandDone(udisksPath);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudDeviceInfo::onCommandError(QDBusError err, QDBusMessage msg)
{
    printf("[YaudDeviceInfo::onCommandError]\n");
    lastError = err.name();
    lastErrDescription = err.message();
    fprintf(stderr, "[ERROR] %s : %s [%d]\n", qPrintable(lastError), qPrintable(lastErrDescription), err.type());
    refreshWidget();
    emit commandError(udisksPath);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
bool YaudDeviceInfo::operator==(const YaudDeviceInfo &yaDI)
{
    bool ret = true;

    if (ret) ret = (udisksPath == yaDI.udisksPath);
    if (ret) ret = (displayName == yaDI.displayName);
    if (ret) ret = (isExternalMountPoint == yaDI.isExternalMountPoint);
    if (ret) ret = (isMounted == yaDI.isMounted);
    if (ret) ret = (isEjectable == yaDI.isEjectable);
    if (ret) ret = (size == yaDI.size);
    if (ret) ret = (fsName == yaDI.fsName);
    if (ret) ret = (mountPath == yaDI.mountPath);
    if (ret) ret = (driveType == yaDI.driveType);

    return ret;
}
