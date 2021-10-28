#include "qtcpserver_rimpl.h"

extern QWaitCondition isShareCompleted;
extern QMutex wMutex;
extern QMutex mutex;
extern QMutex newMutex;
extern QMutex threadListMutex;
extern QMutex finishMutex;



QTcpServer_Rimpl::QTcpServer_Rimpl(QObject *parent):QTcpServer(parent),threadList()
{
    wakeTimerID = startTimer(10);
}


QTcpServer_Rimpl::~QTcpServer_Rimpl()
{
    threadListMutex.lock();
    foreach (ServerThread *theThread, threadList) {
        if(theThread != nullptr){
            theThread->quit();
            theThread->wait();
            delete theThread;
        }
    }
    threadListMutex.unlock();
}



void QTcpServer_Rimpl::incomingConnection(qintptr handle)
{
    newMutex.lock();
    ServerThread *thread = new ServerThread(handle);
    newMutex.unlock();
    emit toEstnewConnection(thread);
}

void QTcpServer_Rimpl::timerEvent(QTimerEvent *event)
{
        if(wakeTimerID == event->timerId()){
            wMutex.lock();
            while(ServerThread::buzy_Count > 0){
                wMutex.lock();
                sleep(1);
                wMutex.unlock();
            }
            finishMutex.lock();
            if(ServerThread::finish_threadCount == threadList.count() || ServerThread::buzy_Count > 0){
                ServerThread::flag_finish_send = true;
                ServerThread::finish_threadCount = 0;
                finishMutex.unlock();
            }
            else {
                finishMutex.unlock();
            }
        isShareCompleted.wakeAll();
        wMutex.unlock();
    }

}
