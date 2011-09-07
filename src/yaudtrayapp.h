#ifndef YAUDTRAYAPP_H
#define YAUDTRAYAPP_H

#include <QApplication>
#include <QDBusConnection>
#include <QDBusObjectPath>

class YaudTrayApp : public QApplication
{
    Q_OBJECT

public:
    explicit YaudTrayApp(int &argc, char **argv);
    virtual ~YaudTrayApp();

private slots:
    void onDeviceAdded(QDBusObjectPath device);
    void onDeviceRemoved(QDBusObjectPath device);
    void onDeviceChanged(QDBusObjectPath device);

private:
    bool connectToUdisks();
    bool getDevicesList();
};

#endif // YAUDTRAYAPP_H
