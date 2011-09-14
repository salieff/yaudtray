#ifndef _YAUD_DEV_INFO_H_
#define _YAUD_DEV_INFO_H_

#include <QString>
#include <QDBusObjectPath>
#include <QWidgetAction>

class DevInfoWidget;

struct YaudDeviceInfo {
    enum DriveType {
        DRT_UNKNOWN,
        DRT_FLASH,
        DRT_CDROM
    };

    YaudDeviceInfo();

    void convert(QDBusObjectPath device);
    void refreshWidget();
    void print();

    bool mount();
    bool unmount();
    bool eject();

    bool operator==(const YaudDeviceInfo &yaDI);

    QString lastError;
    QString lastErrDescription;

    QString udisksPath;
    QString displayName;

    bool isExternalMountPoint;
    bool isMounted;
    bool isEjectable;

    qulonglong size;
    QString fsName;
    QString mountPath;

    DriveType driveType;

    QWidgetAction *menuAction;
    DevInfoWidget *menuWidget;
};

#endif /* _YAUD_DEV_INFO_H_ */
