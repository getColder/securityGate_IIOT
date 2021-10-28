#ifndef TCPTHREAD_TOGATE_H
#define TCPTHREAD_TOGATE_H
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <sctointerface.h>
#include <QDateTime>
#include <QQueue>

class tcpThread_Togate: public QThread
{
    Q_OBJECT
public:
    tcpThread_Togate();
    tcpThread_Togate(QHostAddress, quint16);
    inline bool finished_request() const{return flag_finished_request;}
    ~tcpThread_Togate();

        bool flag = false;
        int color = 0;

private:
    QTcpSocket *tcpSocket = nullptr;
    bool flag_finished_request = false;
    SctoInterFace scto;
    QHostAddress ip;
    quint16 port = 8888;
    QByteArray key;
    int packNumber = 0;
    QByteArray recvBuf;
    QByteArray storeBuf;
    void run() override;
    void initSocket();
    void dealData();
    void reconnect();




public slots:
    void conneteToGate();
    void request();


signals:
    void requesting();
    void established();
    void infoQueue(QByteArray* buf);
    void cmdToSensor(QByteArray) ;
};

#endif // TCPTHREAD_TOGATE_H
