/***************************************************************************
 *   Copyright (C) 2011 by Alexander S. Salieff                            *
 *   salieff@mail.ru                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
