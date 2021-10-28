#ifndef IIOT_G_CLIENT_H
#define IIOT_G_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QRegExp>
#include "tcpthread_togate.h"
#include "infothread.h"



class Iiot_g_client:public QObject
{
    Q_OBJECT
public:
    Iiot_g_client(QObject * parent = nullptr);
    virtual ~Iiot_g_client();

private:
    QTcpSocket *tcpSocket = nullptr;
    tcpThread_Togate *tcpThread;
    InfoThread *iThread;
    void initConncect_qt();


public slots:
    void printfReq();
    void finish_request();
};

#endif // IIOT_G_CLIENT_H
