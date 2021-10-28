#include "iiot_g_server.h"

bool DEBUGflag = false;
bool SAVEflag = false;
Iiot_g_server::Iiot_g_server(QObject *parent) : QObject(parent), tcpServer(this)
  ,spThread(new SerialThread()),ckThread(new checkThread()),uSocket(new QUdpSocket)
{

    QString varStr;
    foreach (QString _var, QCoreApplication::arguments()) {
        varStr += _var;
        varStr += " ";
    }
    QRegExp debugFlag("[\\s]+(-v)[\\s]+");
    debugFlag.indexIn(varStr);
    if(debugFlag.indexIn(varStr) == -1)
        DEBUGflag = false;
    else
        DEBUGflag = true;

    QRegExp saveFlag("[\\s]+(-s)[\\s]+");
    saveFlag.indexIn(varStr);
    if(saveFlag.indexIn(varStr) == -1)
        SAVEflag = false;
    else
        SAVEflag = true;

    printf("\033[1;32m##############################\n");
    printf("#                            #  \n");
    printf("#           Server           #  \n");
    printf("#      listening: 8888       #  \n");
    printf("#                            #  \n");
    printf("##############################\n\n\033[0m");
    init_socket_tcp();
    init_connect();
    ckThread->setThreadList(&tcpServer.threadList);
    spThread->start();
    ckThread->start();


    uSocket->bind(QHostAddress::AnyIPv4,8888);
    uSocket->open(QIODevice::ReadOnly);



}

Iiot_g_server::~Iiot_g_server()
{
    qDebug() << "serial disable.";
    spThread->quit();
    spThread->wait();
    delete spThread;
    delete ckThread;
    delete uSocket;
}



void Iiot_g_server::init_connect()
{
    connect(&tcpServer, &QTcpServer_Rimpl::toEstnewConnection, ckThread, &checkThread::estNewConnection,Qt::QueuedConnection);
    connect(ckThread, &checkThread::tEstablished, spThread, &SerialThread::onEstablished, Qt::QueuedConnection);
    connect(spThread, &SerialThread::ToBuf, ckThread, &checkThread::updateRefToBuf, Qt::QueuedConnection);
    connect(uSocket, &QUdpSocket::readyRead, this,
            [=](){
        int len = uSocket->pendingDatagramSize();
        char buf[len];
        uSocket->readDatagram(buf,len);
        QByteArray cmd(buf,len);
        qDebug() << cmd;
        CMDtoSensor(cmd);
    });
    connect(this, &Iiot_g_server::switchSensor ,spThread, &SerialThread::switchSensor);
    connect(this, &Iiot_g_server::addDeleSensor ,spThread, &SerialThread::addDeleSensor);
}

void Iiot_g_server::init_socket_tcp()
{
    if(!tcpServer.listen(QHostAddress::AnyIPv4,8888))
        qDebug() << "tcpserver listen failed.";
}

void Iiot_g_server::CMDtoSensor(QByteArray( cmd))
{
    if(spThread == nullptr){
        printf("Fatal: Serial is no open\n");
        return;
    }
    if(cmd.contains("#open"))
        emit switchSensor(true);
    else if (cmd.contains("#close"))
        emit switchSensor(false);
    else if(cmd.contains("#add")){
        cmd.remove(0,4);
        if(cmd.size() == 2)
            emit addDeleSensor(cmd,true);
        else
            printf("Try to add sensor,but failed.");
    }
    else if(cmd.contains("#delete")){
        cmd.remove(0,7);
        if(cmd.size() == 2)
            emit addDeleSensor(cmd,false);
        else
            printf("Try to delete sensor,but failed.");
    }
}
