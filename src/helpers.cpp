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
    if (!QIcon::hasThemeIcon(name))
        printf("[yaudIcon] Theme doesn't have icon [%s], embedded fallback will be used\n", qPrintable(name));

    QString fallName = QString(":/icons/") + name + QString(".png");
    QIcon ret = QIcon::fromTheme(name, QIcon(fallName));

    QSize actSize = ret.actualSize(QSize(16, 16));
    if (actSize.width() < 0 ||  actSize.height() < 0)
        printf("[yaudIcon] Returned icon [%s] is invalid!!!\n", qPrintable(name));

    return ret;
}
