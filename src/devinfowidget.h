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

private:
    QString udisksPath;
    QProcess *xdgOpen;

    QString printSize(qulonglong size);

private slots:
    void ejectClicked();
    void mountLinkActivated(QString strLink);
};

#endif
