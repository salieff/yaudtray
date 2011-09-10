#include "devinfo.h"
#include <QDBusInterface>
#include <QStringList>

#include <stdio.h>

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
YaudDeviceInfo::YaudDeviceInfo()
    : isExternalMountPoint(false),
      isMounted(false),
      isEjectable(false),
      size(0),
      driveType(DRT_UNKNOWN),
      fsType(FST_UNKNOWN)
{
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void YaudDeviceInfo::convert(QDBusObjectPath device)
{
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

    if (fsName == "vfat" || fsName == "ntfs")
        fsType = FST_WINDOWS;
    else if (fsName == "iso9660")
        fsType = FST_CDROM;
    else
        fsType = FST_UNIX;


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

    switch (fsType)
    {
    case FST_UNKNOWN :
        printf("    fsType: FST_UNKNOWN\n");
        break;

    case FST_UNIX :
        printf("    fsType: FST_UNIX\n");
        break;

    case FST_CDROM :
        printf("    fsType: FST_CDROM\n");
        break;

    case FST_WINDOWS :
        printf("    fsType: FST_WINDOWS\n");
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

    QDBusMessage mountRes = devIface.call("FilesystemMount", QVariant(QString()), QVariant(QStringList()));
    if (mountRes.type() == QDBusMessage::ErrorMessage)
    {
        lastError = mountRes.errorName();
        lastErrDescription = mountRes.errorMessage();
        fprintf(stderr, "ERROR: Can't call FilesystemMount\n");
        fprintf(stderr, "       %s : %s\n", qPrintable(lastError), qPrintable(lastErrDescription));
        return false;
    }

    return true;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
bool YaudDeviceInfo::unmount()
{
    QDBusInterface devIface("org.freedesktop.UDisks",
                            udisksPath,
                            "org.freedesktop.UDisks.Device",
                            QDBusConnection::systemBus());

    QDBusMessage unmountRes = devIface.call("FilesystemUnmount", QVariant(QStringList()));
    if (unmountRes.type() == QDBusMessage::ErrorMessage)
    {
        lastError = unmountRes.errorName();
        lastErrDescription = unmountRes.errorMessage();
        fprintf(stderr, "ERROR: Can't call FilesystemUnmount\n");
        fprintf(stderr, "       %s : %s\n", qPrintable(lastError), qPrintable(lastErrDescription));
        return false;
    }

    return true;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
bool YaudDeviceInfo::eject()
{
    QDBusInterface devIface("org.freedesktop.UDisks",
                            udisksPath,
                            "org.freedesktop.UDisks.Device",
                            QDBusConnection::systemBus());

    QDBusMessage ejectRes = devIface.call("DriveEject", QVariant(QStringList()));
    if (ejectRes.type() == QDBusMessage::ErrorMessage)
    {
        lastError = ejectRes.errorName();
        lastErrDescription = ejectRes.errorMessage();
        fprintf(stderr, "ERROR: Can't call DriveEject\n");
        fprintf(stderr, "       %s : %s\n", qPrintable(lastError), qPrintable(lastErrDescription));
        return false;
    }

    return true;
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
    if (ret) ret = (fsType == yaDI.fsType);

    return ret;
}
