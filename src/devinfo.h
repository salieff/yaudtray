#ifndef _YAUD_DEV_INFO_H_
#define _YAUD_DEV_INFO_H_

#include <QString>

struct YaudDeviceInfo {
    enum DriveType {
        UNKNOWN,
        FLASH,
        CDROM
    };

    enum FSType {
        UNKNOWN,
        UNIX,
        WINDOWS
    };

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
