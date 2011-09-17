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

#include <stdio.h>
#include "helpers.h"

struct iconForFSStruct {
    const char *fsName;
    const char *fsIcon;
};

iconForFSStruct IconsForFSArray[] = {
    {"vfat", "windows"},
    {"ntfs", "windows"},
    {"iso9660", "media-cdrom"},
    {"hfs", "apple"},
    {"hfsplus", "apple"},
};

QIcon iconForFS(QString fsname)
{
    QString iconName = "linux";

    size_t count = sizeof(IconsForFSArray) / sizeof(iconForFSStruct);
    for (size_t i=0; i<count; ++i)
        if (fsname == IconsForFSArray[i].fsName)
            iconName = IconsForFSArray[i].fsIcon;

    return yaudIcon(iconName);
}

QIcon yaudIcon(QString name)
{
    QString fallName = QString(":/icons/") + name + QString(".png");

#if QT_VERSION >= 0x040600
    if (!QIcon::hasThemeIcon(name))
        printf("[yaudIcon] Theme doesn't have icon [%s], embedded fallback will be used\n", qPrintable(name));

    QIcon ret = QIcon::fromTheme(name, QIcon(fallName));
#else
    QIcon ret = QIcon(fallName);
#endif

    QSize actSize = ret.actualSize(QSize(16, 16));
    if (actSize.width() < 0 ||  actSize.height() < 0)
        printf("[yaudIcon] Returned icon [%s] is invalid!!!\n", qPrintable(name));

    return ret;
}
