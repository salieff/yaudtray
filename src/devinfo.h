#ifndef _YAUD_DEV_INFO_H_
#define _YAUD_DEV_INFO_H_

#include <QObject>
#include <QString>
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QDBusError>
#include <QWidgetAction>

class DevInfoWidget;

class YaudDeviceInfo : public QObject
{

    Q_OBJECT

public :
    enum DriveType {
        DRT_UNKNOWN,
        DRT_FLASH,
        DRT_CDROM
    };

    YaudDeviceInfo();
    YaudDeviceInfo(QDBusObjectPath device);

    void convert(QDBusObjectPath device);
    void refreshWidget();
    void print();

    bool mount();
    bool unmount();
    bool eject();

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

    DevInfoWidget *menuWidget;

signals :
    void commandError(QString udisksPath);
    void commandDone(QString udisksPath);

private slots :
    void onCommandDone(QDBusMessage msg);
    void onCommandError(QDBusError err, QDBusMessage msg);

private :
    void reset();
};

#endif /* _YAUD_DEV_INFO_H_ */
