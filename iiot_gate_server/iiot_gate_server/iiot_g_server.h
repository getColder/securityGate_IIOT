#ifndef IIOt_G_SERVER_H
#define IIOt_G_SERVER_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QCoreApplication>
#include "serialthread.h"
#include "qtcpserver_rimpl.h"
#include "checkthread.h"



class Iiot_g_server : public QObject
{
    Q_OBJECT
public:
    explicit Iiot_g_server(QObject *parent = nullptr);
    ~Iiot_g_server();
private:
    QTcpServer_Rimpl tcpServer;
    SerialThread *spThread;
    checkThread *ckThread;
    QUdpSocket *uSocket;
    void init_socket_tcp();
    void init_connect();
    void CMDtoSensor(QByteArray cmd);

signals:
    void switchSensor(bool ok);
    void addDeleSensor(QByteArray,bool);

public slots:

};

#endif // Iiot_G_SERVER_H
