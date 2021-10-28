#include "iiot_g_client.h"

int PIDcolor = 0;
bool DEBUGflag = false;

Iiot_g_client::Iiot_g_client(QObject *parent):QObject(parent),iThread(new InfoThread())
{
    QString varStr;
    foreach (QString _var, QCoreApplication::arguments()) {
        varStr += _var;
        varStr += " ";
    }
    QRegExp color("[c][\\s]+([\\d])");
    QRegExp IPstr("[a][\\s]+([\\d]{1,3}[/.][\\d]{1,3}[/.][\\d]{1,3}[/.][\\d]{1,3})[:]([\\d]{1,5})");
    color.indexIn(varStr);
    IPstr.indexIn(varStr);
    QHostAddress inputAddress(IPstr.cap(1));
    quint16 _port =  IPstr.cap(2).toUInt();
    PIDcolor = color.cap(1).isEmpty()?0:color.cap(1).toUInt() % 7;
    QRegExp debugFlag("[\\s]+(-v)[\\s]+");
    debugFlag.indexIn(varStr);
    if(debugFlag.indexIn(varStr) == -1)
        DEBUGflag = false;
    else
        DEBUGflag = true;

    if(inputAddress.toString() != "" && _port > 0){
        tcpThread = new tcpThread_Togate(inputAddress , _port);
        tcpThread->start();
    }
    else{
        tcpThread = new tcpThread_Togate();
        tcpThread->start();
    }

    setbuf(stdout,NULL);
    initConncect_qt();
    iThread->start();
}

Iiot_g_client::~Iiot_g_client()
{
    tcpThread->quit();
    tcpThread->wait();
    delete tcpThread;
    iThread->quit();
    iThread->wait();
    delete iThread;
}


void Iiot_g_client::initConncect_qt()
{
    connect(tcpThread, &tcpThread_Togate::requesting, this, &Iiot_g_client::printfReq,Qt::QueuedConnection);
    connect(tcpThread, &tcpThread_Togate::established, this, [=](){
        printf("Established!\n");},Qt::QueuedConnection);
    connect(tcpThread, &tcpThread_Togate::infoQueue, iThread, &InfoThread::getInfo, Qt::QueuedConnection);
}


void Iiot_g_client::printfReq()
{
    printf("requesting");
    while(!tcpThread->finished_request()){
        for(int i = 0; i < 4; i++){
            printf(".");
            sleep(1);
        }
        printf("\b\b\b\b");
        printf("    ");
        printf("\b\b\b\b");
    }
    printf("\n");
}

void Iiot_g_client::finish_request()
{

}
