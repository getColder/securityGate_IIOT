#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H
#include <QObject>
#include <QEvent>
#include <QTimerEvent>
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include "sctointerface.h"
#include "patterns.h"
#include <QFile>

class ServerThread:public QThread
{
    Q_OBJECT
private:
    qintptr socketDescriptor;
    QHostAddress peer;
    SctoInterFace scto;
    QByteArray sm4_key;
    QByteArray buf;
    static QByteArray p_data;
    QQueue<QByteArray> *p_stream = nullptr;
    static Patterns m_patterns;
    int timerID = -1;
    int packNumber = 0;
    ServerThread();
    void run() override;
    void execRequest();
    void request();
    void timerEvent(QTimerEvent *event) override;
    void sendStream();

    QFile *file = nullptr;




public:
    ServerThread(qintptr _socketDescriptor);
    ~ServerThread();
    int numberId;
    bool flag_stream_enable = false;
    static int finish_threadCount;
    static int buzy_Count;
    static bool flag_finish_send;
    QTcpSocket *socket;
    void setPStream(QQueue<QByteArray>* _p){if(_p != nullptr)p_stream = _p;};
    void setNUmberID(int numberID);


public slots:
    void listenRequest();
    void stopTimer(int numberID);

signals:
    void quitMe();
    void established();
    void start_send();
    void alive();

};

#endif // SERVERTHREAD_H
