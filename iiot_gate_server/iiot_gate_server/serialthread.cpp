#include "serialthread.h"

extern QMutex w;
extern QMutex lockMutex;


SerialThread::SerialThread()
{
    moveToThread(this);
}

SerialThread::~SerialThread()
{
}

void SerialThread::run()
{
    sp = new QSerialPort();
    init_serial();
    connect(sp, &QSerialPort::readyRead, [=](){
        w.lock();
        QByteArray data;
        data = sp->readAll();
        if(!tcpbuf_Serial_dataList.isEmpty())
            tcpbuf_Serial_dataList.dequeue();
        tcpbuf_Serial_dataList.enqueue(data);
        w.unlock();
    });
    exec();
}



void SerialThread::init_serial()
{
    sp->setPortName("/dev/ttyAMA0");
    sp->setParity(QSerialPort::NoParity);
    sp->setBaudRate(QSerialPort::Baud115200);
    sp->setDataBits(QSerialPort::Data8);
    sp->setStopBits(QSerialPort::OneStop);
    sp->setFlowControl(QSerialPort::NoFlowControl);
    if(!sp->open(QIODevice::ReadWrite))
        printf("serial port open failed;\n");
    else{
        printf("serial port initialized successfully!\n");
    }
}

void SerialThread::onEstablished()
{
    emit ToBuf(&tcpbuf_Serial_dataList);
}

void SerialThread::switchSensor(bool s)
{
    if(s == true)
        sp->write("{RCC}");
    else
        sp->write("{SCC}");
}

void SerialThread::addDeleSensor(QByteArray _idNumber,bool s)
{
    char op = '\x2d';
    if(s == true)
        op = '\x2b';
    else
        op = '\x2d';
    QByteArray cmdTosensors("{");
    cmdTosensors.append(op);
    cmdTosensors.append(_idNumber);
    cmdTosensors.append("}");
    sp->write(cmdTosensors);
}
