#ifndef _YAUD_DEV_INFO_H_
#define _YAUD_DEV_INFO_H_

#include <QString>
#include <QDBusObjectPath>
 
struct YaudDeviceInfo {
    enum DriveType {
        DRT_UNKNOWN,
        DRT_FLASH,
        DRT_CDROM
    };

    enum FSType {
        FST_UNKNOWN,
        FST_UNIX,
        FST_CDROM,
        FST_WINDOWS
    };
	
	YaudDeviceInfo();
	bool convert(QDBusObjectPath device);
	
    QString udisksPath;
    QString displayName;

    bool isMounted;
    bool isEjectable;

    qulonglong size;
    QString fsName;
    QString mountPath;

    DriveType driveType;
    FSType fsType;
};

#endif /* _YAUD_DEV_INFO_H_ */
