#ifndef INFOTHREAD_H
#define INFOTHREAD_H
#include <QThread>
#include <QQueue>
#include <QTimerEvent>
#include <QUdpSocket>
#include <QMutex>

class InfoThread:public QThread
{
    Q_OBJECT
public:
    InfoThread();
    ~InfoThread();

private:
    void run() override;
    QByteArray *infoQue = nullptr;
    quint16 uPort = 9527;
    quint16 u_ServerPort = 8888;
    QHostAddress uIP;
    QHostAddress uServerIP;
    QUdpSocket *usocket;
    int enqueTimerID = -1;
    int infoTimerID = -1;
    int errorCount = 0;
    void timerEvent(QTimerEvent *event) override;

    bool sensorOpen = false;
public slots:
    void getInfo(QByteArray *p);


};

#endif // INFOTHREAD_H
