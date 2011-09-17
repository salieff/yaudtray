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

#ifndef _YAUD_DEV_INFO_WIDGET_H_
#define _YAUD_DEV_INFO_WIDGET_H_

#include <QProcess>

#include "ui_devinfowidget.h"

class YaudDeviceInfo;

class DevInfoWidget : public QWidget, public Ui::DevInfoWidget
{
    Q_OBJECT

public:
    explicit DevInfoWidget(YaudDeviceInfo *yaudDI, QWidget *parent = NULL);
    virtual ~DevInfoWidget();

    void convertFrom(YaudDeviceInfo *yaudDI);

signals:
    void requestProcessing(QString udisksPath);
    void requestCloseError(QString udisksPath);

private:
    QString udisksPath;
    QProcess *xdgOpen;

    QString printSize(qulonglong size);

private slots:
    void ejectClicked();
    void errCloseClicked();
    void mountLinkActivated(QString strLink);
};

#endif
