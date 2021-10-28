#include "checkthread.h"


extern QMutex newMutex;
extern QMutex threadListMutex;

checkThread::checkThread()
{
    moveToThread(this);
}

void checkThread::run()
{
    for(int i = 0; i < 200; i++){
        alive[i] = 0;
        zombie[i] = 0;
    }
    cleanTimerID = startTimer(5000);
    plusTimerID = startTimer(1000);
    exec();
}

void checkThread::estNewConnection(ServerThread* newThread)
{
    threadListMutex.lock();
    if(findAvalibleNumberID() != -1){
        newThread->setNUmberID(numberID);
        threadList->insert(numberID,newThread);
    }
    else{
        delete newThread;
        return;
    }
    threadListMutex.unlock();
    connect(newThread, &ServerThread::destroyed, [=](){
        if(alive[newThread->numberId] > 0)
            zombie[newThread->numberId] = 1;
    });
    connect(newThread, &ServerThread::quitMe, this,[=](){
        if(alive[newThread->numberId] > 0)zombie[newThread->numberId] = 1;});
    connect(newThread, &ServerThread::established, this, [=](){
        emit tEstablished();
    }, Qt::QueuedConnection);
    connect(newThread, &ServerThread::alive, this, [=](){
        alive[newThread->numberId] = 1;
    }, Qt::QueuedConnection);
    connect(this,  &checkThread::stopT, newThread, &ServerThread::stopTimer , Qt::QueuedConnection);
    newThread->start();
}

void checkThread::updateRefToBuf(QQueue<QByteArray> *bufList)
{
    threadListMutex.lock();
    foreach(ServerThread* theThread, *threadList){
        if(theThread != nullptr){
            theThread->setPStream(bufList);
            theThread->flag_stream_enable = true;
        }
    }
    threadListMutex.unlock();
}



void checkThread::timerEvent(QTimerEvent *event)
{
    if(plusTimerID == event->timerId())
        printf("-\n");

    if(cleanTimerID == event->timerId()){
        int currentAliveNumber = 0;
        for(int i = 0; i < 200; i++){
            if(alive[i] > 0){
                ++currentAliveNumber;
                i ++;
            }
            if(alive[i] > 10){
                zombie[i] = 1;
            }
        }
        for (int i = 0; i < 200; i++) {
            if( zombie[i] == 1){
                ServerThread *pT;
                threadListMutex.lock();
                pT = threadList->value(i);
                zombie[i] = 0;
                qDebug() << "ready to release:" << pT << "number" << i;
                if(pT != nullptr){
                    qDebug() << pT;
                    alive[i] = 0;
                    zombie[i] = 0;
                    emit stopT(pT->numberId);
                    pT->quit();
                    pT->wait();
                    delete pT;
                    pT = nullptr;
                    threadList->remove(i);
                }
                threadListMutex.unlock();
            }
        }
        aliveNumber = currentAliveNumber;
    }

}

int checkThread::findAvalibleNumberID()
{
    for(int i = 0;i < 128; i++){
        if(!threadList->contains(i)){
            numberID = i;
            zombie[i] = 0;
            break;
        }
        else{
            if(i < 127)
                continue;
            else{
                printf("128 thread running,no available any more\n");
                return -1;
            }
        }
    }
    return numberID;
}
