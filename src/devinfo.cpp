#include "devinfo.h"
#include <QDBusInterface>
#include <QStringList>

#include <stdio.h>

YaudDeviceInfo::YaudDeviceInfo()
	: isMounted(false),
	  isEjectable(false),
	  size(0),
	  driveType(DRT_UNKNOWN),
	  fsType(FST_UNKNOWN)
{
}

bool YaudDeviceInfo::convert(QDBusObjectPath device)
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
	
#define PRINT_STRING(arg) printf("    "#arg": %s\n", qPrintable(arg));
#define PRINT_BOOL(arg) printf("    "#arg": %s\n", arg ? "TRUE" : "FALSE");
#define PRINT_ULL(arg) printf("    "#arg": %llu\n", arg);
    PRINT_STRING(udisksPath)
    PRINT_STRING(displayName)
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

    if (devIface.property("DeviceIsSystemInternal").toBool())
    	return false;
    	
    if (!devIface.property("DeviceIsMediaAvailable").toBool())
    	return false;
    	
    if (devIface.property("IdUsage").toString() != "filesystem")
    	return false;

	return true;
}
