#include "infothread.h"

extern QMutex queMutex;
InfoThread::InfoThread():uIP("0"),uServerIP("127.0.0.1")
{

    moveToThread(this);
}

void InfoThread::run()
{
    usocket = new QUdpSocket();
    usocket->bind(QHostAddress::AnyIPv4, uPort);
    usocket->open(QIODevice::ReadWrite);
    connect(usocket, &QUdpSocket::readyRead,[=](){
        const quint64 len = usocket->pendingDatagramSize();
        char arry[len];
        usocket->readDatagram(arry,len);
        QByteArray readData(arry,len);
        if(readData.contains("sync")){
            printf("\nSync successfully\n");
            readData.remove(0,4);
            uIP = QHostAddress(QString(readData.data()));
            infoTimerID = startTimer(10);
        }
        else if(readData.contains("#open")){
            qDebug() << readData;
            usocket->writeDatagram("#open", 5, uServerIP, u_ServerPort);
        }
        else if(readData.contains("#close")){
            qDebug() << readData;
            usocket->writeDatagram("#close", 6, uServerIP, u_ServerPort);
        }
        else if(readData.contains("#add")){
            qDebug() << readData;
            usocket->writeDatagram(readData, readData.size(), uServerIP, u_ServerPort);
        }
        else if(readData.contains("#delete")){
            qDebug() << readData;
            usocket->writeDatagram(readData, readData.size(), uServerIP, u_ServerPort);
        }
    });
    exec();
}

InfoThread::~InfoThread()
{
    delete usocket;
}

void InfoThread::getInfo(QByteArray *p)
{
    infoQue = p;
}

void InfoThread::timerEvent(QTimerEvent *event)
{
    if(infoTimerID == event->timerId()){
        queMutex.lock();
        if(!infoQue->isEmpty()){
            int count = 0;
            int start = 0;
            int end = 0;
            start = infoQue->indexOf('{', count);
            while(start > 0){
                infoQue->remove(0,1);
            }
            if(start == -1){
                infoQue->clear();
                queMutex.unlock();
                return;
            }
            int next7d = 0;
            while(end < 8){
                int temp = infoQue->indexOf('}',count + next7d);
                if(temp == -1)
                    break;
                if(temp - start == 1)
                    break;
                end = temp;
                next7d ++;
//                QByteArray whatTheFuckingWrongWithFrameFromSTM32 = infoQue->mid(start, end - start + 1);
//                if(!(whatTheFuckingWrongWithFrameFromSTM32.contains("OK+")
//                     || whatTheFuckingWrongWithFrameFromSTM32.contains("OK-")
//                     ||whatTheFuckingWrongWithFrameFromSTM32.contains("OKR")
//                     ||  whatTheFuckingWrongWithFrameFromSTM32.contains("OKS")
//                     ))
//                    infoQue->remove(start, end - start + 2);
//                else
//                    break;
            }
            if(end == -1){
                queMutex.unlock();
                return;
            }
            QByteArray temp(infoQue->mid(start,8));
            usocket->writeDatagram(temp,temp.size(),uIP,9988);
            infoQue->remove(start, end - start + 1);
        }
            queMutex.unlock();
    }
    else {
        queMutex.unlock();
    }
}
