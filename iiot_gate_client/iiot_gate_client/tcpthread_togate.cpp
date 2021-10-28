#include "tcpthread_togate.h"
static char originKey[16] = {'a','k','e', 'y', 'h', 'm', 'a', 'c'};
static uint8_t iv[16] = { 0xC7,0x2B,0x65,0x91,0xA0,0xD7,0xDE,0x8F,0x6B,0x40,0x72,0x33,0xAD,0x35,0x81,0xD6};
extern QMutex queMutex;
extern int PIDcolor;
extern bool DEBUGflag;

union semun{
    int val;
};
static int id = semget(1243,1,IPC_CREAT|0644);
void P(int id)
{
    struct sembuf sb[1];
    sb[0].sem_num=0;
    sb[0].sem_op=-1;
    sb[0].sem_flg=0;
    semop(id,sb,1);
}

void V(int id)
{
    struct sembuf sb[1];
    sb[0].sem_num= 0;
    sb[0].sem_op= 1;
    sb[0].sem_flg= 0;
    semop(id,sb,1);
}

tcpThread_Togate::tcpThread_Togate():scto(),ip("192.168.1.128"),key(),recvBuf()
{
    moveToThread(this);
}

tcpThread_Togate::tcpThread_Togate(QHostAddress _ip, quint16 _port):scto(),ip(_ip), port(_port),key()
{
    moveToThread(this);
}

tcpThread_Togate::~tcpThread_Togate()
{
    if(tcpSocket != nullptr){
        tcpSocket->disconnectFromHost();
        tcpSocket = nullptr;
    }
}

void tcpThread_Togate::run()
{
    QString localIP;
    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol && !address.toString().contains("127.0."))
            localIP = address.toString();
    };
    printf("\033[1;33m####################\n");
    printf("#                  #  \n");
    printf("#      Client      #  \n");
    QString infoHead =  "#  " + localIP + "   #";
    qDebug() << infoHead.toUtf8().data();
    printf("#                  #  \n");
    printf("####################\n\n\033[0m");
    initSocket();
    emit infoQueue(&storeBuf);
    conneteToGate();
    exec();
}


void tcpThread_Togate::conneteToGate()
{
    tcpSocket->connectToHost(QHostAddress(ip),port);
    while(!tcpSocket->waitForConnected()){
        printf("\nNo respond!\n");
        emit tcpSocket->disconnected();
    }
}

void tcpThread_Togate::request()
{
    if(tcpSocket == nullptr){
        printf("Failed! No connection!\n");
        return;
    }
    if(tcpSocket->state() != QAbstractSocket::ConnectedState){
        printf("Failed to connecte!\n");
        return;
    }

    /* request */
    printf("trying to request the stream!\n");
    emit requesting();
    tcpSocket->write("#c#request###");
    /* send pubkey */
    QByteArray readBuf;
    while(tcpSocket->waitForReadyRead(1500)){
        readBuf += tcpSocket->readAll();
        if(readBuf.size() > 1000){
            printf("exceptional respond.\n");
            flag_finished_request = true;
            tcpSocket->disconnectFromHost();
            return;      // unexpected stream
        }
        if(readBuf.contains("ok")){
            readBuf.remove(readBuf.indexOf("ok"), 2);
            break;
        }
        if(readBuf.contains("wrong")){
            printf("Fatal: request wrong!");
            return;
        }
    }
    /* get pubKey */
    QByteArrayList key2;
    P(id);
    if(scto.open()){
        while(!scto.isOpen()){
            scto.open();
            sleep(1);
        }
        key2 = scto.newKey(SctoInterFace::SM2_KEY);
        scto.setMODE(SctoInterFace::SM3MODE);
        QByteArray digest_HMAC(scto.encrypt_HMAC(QByteArray(originKey).data(),key2.first().data(), key2.first().size()));
        V(id);
        tcpSocket->write(QByteArray("#d#" + key2.first() + "##%"));
        tcpSocket->write("#d#" + digest_HMAC + "##%");
    }
    else{
        flag_finished_request = true;
        printf("\nFatal: scto cannot open!\n");
        sleep(30);
        tcpSocket->disconnectFromHost();
        return;
    }
    while(tcpSocket->waitForReadyRead(15000)){

        readBuf += tcpSocket->readAll();
        if(readBuf.size() > 1000){
            printf("exceptional respond.\n");
            flag_finished_request = true;
            tcpSocket->disconnectFromHost();
            return;      // unexpected stream
        }
        if(readBuf.contains("wrong")){
            printf("Fatal: request wrong!");
            return;
        }
        if(readBuf.count("###") == 2){
            flag_finished_request = true;
            readBuf.remove(readBuf.indexOf("###",0),3);
            readBuf.remove(readBuf.indexOf("###",0),3);
            scto.setMODE_SM2(key2.first(),key2.last());
            key = scto.decrypt(readBuf.data(), readBuf.size()).first();
            scto.open();
            emit established();
            packNumber = 0;
            while(tcpSocket->waitForReadyRead(-1)){
                recvBuf = tcpSocket->readAll();        /* deal data stream   */
                dealData();
            }
        }
    }
}

void tcpThread_Togate::initSocket()
{
    if(tcpSocket != nullptr){
        printf("Failed! Tcp socket exists\n");
        return;
    }
    tcpSocket = new QTcpSocket();
    connect(tcpSocket, &QTcpSocket::connected, [=](){
        request();
    });
    connect(tcpSocket, &QTcpSocket::disconnected, [=](){
        sleep(1);
        printf("Server has disconnected from host.\n");
        flag_finished_request = true;
        tcpSocket->deleteLater();
        tcpSocket = nullptr;
        reconnect();
    });

}

void tcpThread_Togate::reconnect()
{
    if(tcpSocket != nullptr){
        delete tcpSocket;
        tcpSocket = nullptr;
    }
    initSocket();
    while(1){
        if(tcpSocket->state() == QAbstractSocket::UnconnectedState)
            tcpSocket->connectToHost(QHostAddress(ip),port);
        if(!tcpSocket->waitForConnected(1000))
            sleep(1);
        if(tcpSocket->state() == QAbstractSocket::ConnectedState)
            return;
        printf("Waiting for reconnecting");
        sleep(1);
        printf(".");
        sleep(1);
        printf(".");
        sleep(1);
        printf(".");
        sleep(1);
        printf(".");
        sleep(1);
        printf("\b\b\b\b");
        printf("    ");
        printf("\b\b\b\b\nTry  again..\n");
        sleep(3);
        int count = 0;
        while(count < 5)
        {
            if(tcpSocket->state() == QAbstractSocket::ConnectedState)
                return;
            sleep(1);
            count ++;
        }
        tcpSocket->abort();
        qDebug() << "the Host not reachable...\n";
    }
}


void tcpThread_Togate::dealData()
{
    int iStart = 0;
    int iEnd = 0;
    if( recvBuf.isEmpty() )
        return;
    while(((iStart = recvBuf.indexOf("#d#",0)) != -1) && ((iEnd = recvBuf.indexOf("%#%",0)) != -1)){
        QByteArray currentData = recvBuf.mid(iStart + 3, iEnd - iStart - 3);
        recvBuf.remove(iStart, iEnd - iStart + 3);
        if( (currentData.at(0) - 48) != (packNumber %= 10)){
            printf("wrong order");
            tcpSocket->disconnectFromHost();
        }
        currentData.remove(0,1);
        P(id);
        if(!scto.isOpen())
            scto.open();
        scto.setMODE_SM4(key,QByteArray((char*)iv,16));
        QByteArrayList plain = scto.decrypt(currentData.data(),currentData.size());
        V(id);
        queMutex.lock();
        int showLen = 0;
        foreach(QByteArray _plainData, plain){
            showLen += _plainData.size();
            storeBuf = _plainData;
            if(DEBUGflag == true)
                qDebug() << _plainData.toHex('-');
        }
        queMutex.unlock();
        qDebug() << QDateTime::currentDateTime().toString("ddd, HH:mm:ss AP") + QString(" .Number[%1] read %2 from ")
                    .arg(packNumber).arg(showLen) << QString("\033[1;%1m").arg(PIDcolor + 31).toLocal8Bit().data() << QThread::currentThreadId()
                 << "\033[0m";
        ++packNumber;
    }
    return;
}
