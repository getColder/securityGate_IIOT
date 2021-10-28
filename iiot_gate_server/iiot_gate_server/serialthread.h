#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QObject>
#include <QThread>
#include <QEvent>
#include <QQueue>
#include <QSerialPort>
#include <QMutex>
#include <QTimer>
#include <QDebug>
#include <QDateTime>


class SerialThread:public QThread
{
    Q_OBJECT
public:
    SerialThread();
    ~SerialThread();
    void run() override;


private:
    QSerialPort *sp;
    QByteArray tcp;
    QQueue<QByteArray> tcpbuf_Serial_dataList;
    void init_serial();


public slots:
    void onEstablished();
    void switchSensor(bool s);
    void addDeleSensor(QByteArray id,bool s);

signals:
    void ToBuf(QQueue<QByteArray> *buflist);

};

#endif // SERIALTHREAD_H
