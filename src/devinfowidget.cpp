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

#include <QDesktopServices>
#include <QUrl>
#include <QProcess>

#include "devinfowidget.h"
#include "devinfo.h"
#include "helpers.h"

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
DevInfoWidget::DevInfoWidget(YaudDeviceInfo *yaudDI, QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    convertFrom(yaudDI);

    ejectButton->setText("");
    connect(ejectButton, SIGNAL(clicked()), this, SLOT(ejectClicked()));
    connect(errCloseButton, SIGNAL(clicked()), this, SLOT(errCloseClicked()));

    xdgOpen = new QProcess(this);

    errIconlabel->setPixmap(yaudIcon("dialog-warning").pixmap(22, 22));
    errCloseButton->setIcon(yaudIcon("window-close"));
    errCloseButton->setText("");
    errGroupBox->hide();

    connect(mountPathLabel, SIGNAL(linkActivated(QString)), this, SLOT(mountLinkActivated(QString)));
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
DevInfoWidget::~DevInfoWidget()
{
}

// --------========++++++++ooooooooOOOOOOOOoooooooo++++++++========--------
void DevInfoWidget::convertFrom(YaudDeviceInfo *yaudDI)
{

    QString displayName = yaudDI->displayName;

    /*
    if (displayName.length() > 16)
    {
        displayName = yaudDI->displayName.left(7) + QString("...") + yaudDI->displayName.right(7);
        nameLabel->setToolTip(yaudDI->displayName);
    }
    */

    nameLabel->setText(displayName + QString(" (") + printSize(yaudDI->size) + QString(")"));

    switch(yaudDI->driveType)
    {
    case YaudDeviceInfo::DRT_CDROM :
        diskTypeLabel->setPixmap(yaudIcon("drive-cdrom").pixmap(48, 48));
        break;

    default :
        diskTypeLabel->setPixmap(yaudIcon("drive-removable-media-usb").pixmap(48, 48));
        break;
    }

    OsLabel->setPixmap(iconForFS(yaudDI->fsName).pixmap(22, 22));

    if (yaudDI->isMounted)
    {
        ejectButton->setIcon(yaudIcon("media-eject"));

        QString displayPath = yaudDI->mountPath;

        /*
        if (displayPath.length() > 26)
        {
            displayPath = yaudDI->mountPath.left(12) + QString("...") + yaudDI->mountPath.right(12);
            mountPathLabel->setToolTip(yaudDI->mountPath);
        }
        */

        QString mountUrl = "<A HREF=\"" + yaudDI->mountPath + "\">" + displayPath + "</A>";
        mountPathLabel->setText(mountUrl);
    }
    else
    {
        ejectButton->setIcon(yaudIcon("system-run"));
        mountPathLabel->setText(tr("Not mounted"));
    }

    if (yaudDI->lastError.isEmpty())
    {
        errGroupBox->hide();
    }
    else
    {
        errNameLabel->setText(yaudDI->lastError);
        errTextLabel->setText(yaudDI->lastErrDescription);
        errGroupBox->show();
        errGroupBox->updateGeometry();
    }

    udisksPath = yaudDI->udisksPath;

    updateGeometry();
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
void DevInfoWidget::errCloseClicked()
{
    emit requestCloseError(udisksPath);
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
