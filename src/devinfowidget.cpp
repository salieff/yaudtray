#include <stdio.h>

#include <QDesktopServices>
#include <QUrl>
#include <QProcess>

#include "devinfowidget.h"
#include "devinfo.h"

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
DevInfoWidget::DevInfoWidget(YaudDeviceInfo *yaudDI, QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    convertFrom(yaudDI);

    connect(ejectButton, SIGNAL(clicked()), this, SLOT(ejectClicked()));

    xdgOpen = new QProcess(this);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
DevInfoWidget::~DevInfoWidget()
{
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void DevInfoWidget::convertFrom(YaudDeviceInfo *yaudDI)
{
    nameLabel->setText(yaudDI->displayName + QString(" (") + printSize(yaudDI->size) + QString(")"));
    ejectButton->setText("");

    switch(yaudDI->driveType)
    {
    case YaudDeviceInfo::DRT_CDROM :
        diskTypeLabel->setPixmap(QIcon::fromTheme("drive-cdrom").pixmap(48, 48));
        break;

    default :
        diskTypeLabel->setPixmap(QIcon::fromTheme("drive-removable-media-usb").pixmap(48, 48));
        break;
    }

    switch(yaudDI->fsType)
    {
    case YaudDeviceInfo::FST_WINDOWS :
        OsLabel->setPixmap(QIcon::fromTheme("windows", QIcon(":/icons/windows.png")).pixmap(22, 22));
        break;

    case YaudDeviceInfo::FST_CDROM :
        OsLabel->setPixmap(QIcon::fromTheme("media-cdrom").pixmap(22, 22));
        break;

    case YaudDeviceInfo::FST_APPLE :
        OsLabel->setPixmap(QIcon::fromTheme("apple", QIcon(":/icons/apple.png")).pixmap(22, 22));
        break;

    default :
        OsLabel->setPixmap(QIcon::fromTheme("linux", QIcon(":/icons/linux.png")).pixmap(22, 22));
        break;
    }

    if (yaudDI->isMounted)
    {
        ejectButton->setIcon(QIcon::fromTheme("media-eject"));

        QString displayPath = yaudDI->mountPath;
        if (displayPath.length() > 26)
        {
            displayPath = yaudDI->mountPath.left(12) + QString("...") + yaudDI->mountPath.right(12);
            mountPathLabel->setToolTip(yaudDI->mountPath);
        }

        QString mountUrl = "<A HREF=\"" + yaudDI->mountPath + "\">" + displayPath + "</A>";
        mountPathLabel->setText(mountUrl);
        connect(mountPathLabel, SIGNAL(linkActivated(QString)), this, SLOT(mountLinkActivated(QString)));
    }
    else
    {
        ejectButton->setIcon(QIcon::fromTheme("system-run"));
        mountPathLabel->setText(tr("Not mounted"));
        disconnect(mountPathLabel, SIGNAL(linkActivated(QString)), NULL, NULL);
    }

    udisksPath = yaudDI->udisksPath;
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
QString DevInfoWidget::printSize(qulonglong size)
{
    if (size < 1024)
        return QString(tr("%1 b")).arg(size);

    if (size >= 1024 && size < 1048576)
        return QString(tr("%1 Kb")).arg((double)size/1024, 0, 'f', 1);

    if (size >= 1048576 && size < 1073741824)
        return QString(tr("%1 Mb")).arg((double)size/1048576, 0, 'f', 1);

    return QString(tr("%1 Gb")).arg((double)size/1073741824, 0, 'f', 1);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void DevInfoWidget::ejectClicked()
{
    emit requestProcessing(udisksPath);
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void DevInfoWidget::mountLinkActivated(QString strLink)
{
//	QDesktopServices::openUrl(QUrl(strLink, QUrl::TolerantMode));
//	QDesktopServices::openUrl(QUrl::fromLocalFile(strLink));
//	QDesktopServices::openUrl(QUrl::fromUserInput(strLink));

    QString cmd = QString("xdg-open \"") + strLink + QString("\"");
    printf("CMD: %s\n", qPrintable(cmd));
    xdgOpen->start(cmd);
}
