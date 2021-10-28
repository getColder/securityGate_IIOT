#include "serverthread.h"
#include <QTimer>

extern char originKey[16];
extern char iv[16];
extern bool DEBUGflag;
extern bool SAVEflag;
extern QMutex mutex;
extern QMutex w;
extern QMutex wMutex;
extern QMutex newMutex;
extern QMutex finishMutex;
extern QMutex lockMutex;
extern QWaitCondition isShareCompleted;
int ServerThread::buzy_Count = 0;
int ServerThread::finish_threadCount = 0;
bool ServerThread::flag_finish_send = false;
Patterns ServerThread::m_patterns;
QByteArray ServerThread::p_data;


extern int id;
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



ServerThread::ServerThread(qintptr _socketDescriptor):socketDescriptor(_socketDescriptor),scto()
{
    emit alive();
    moveToThread(this);

}

void ServerThread::setNUmberID(int _numberID)
{
    numberId = _numberID;
}

void ServerThread::run()
{
    newMutex.lock();
    socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);
    peer =  socket->peerAddress();
    qDebug() << "\n\nnew connection! " << peer << QString("in thread[%1]").arg(numberId);
    connect(socket, &QTcpSocket::readyRead, this, &ServerThread::listenRequest);
    connect(socket, &QTcpSocket::disconnected, [=](){ printf("Connection Lost in THREAD [%d]\n",numberId);
        killTimer(timerID);emit quitMe();});
    newMutex.unlock();
    if(numberId == 0 && SAVEflag == true){
        file = new QFile("/opt/cipherFile");
        qDebug() << file->open(QIODevice::ReadWrite|QIODevice::Append);
    }
    exec();
}

ServerThread::~ServerThread()
{
    qDebug() << peer << QString("client quint from thread[%1]").arg(numberId);
    delete socket;
    if(file != nullptr){
        file->close();
        delete file;
    }
}

void ServerThread::stopTimer(int _numberID)
{
    if(_numberID == numberId)
        killTimer(timerID);
}

void ServerThread::listenRequest()
{
    buf += socket->readAll();
    m_patterns.filter(buf);
    execRequest();

}

void ServerThread::execRequest()
{
    QQueue<QString> &cmdList = m_patterns.getCmdList();
    while(!cmdList.isEmpty()){
        QString Cmd = cmdList.dequeue();
        if(Cmd == "request")
            request();
    }
}

void ServerThread::request()
{
    if(!socket->write("ok"))
        qDebug() << "Failed to try to reply!";
    bool flag_getPubKey = false;
    QQueue<QByteArray> &dataList = m_patterns.getDataList();
    while(socket->waitForReadyRead(10000)){
        printf("verify the pub key...\n");
        buf += socket->readAll();
        m_patterns.filter(buf);
        QByteArray pubKey;
        if(!dataList.isEmpty() && flag_getPubKey == false){
            pubKey = dataList.dequeue();
            if(pubKey.size() == 65){
                flag_getPubKey = true;
            }
            else
                printf("Fata: pubkey is invalid.");
        }
        if(!dataList.isEmpty() && flag_getPubKey == true){
            P(id);
            while(!scto.isOpen())
                scto.open();
            scto.setMODE(SctoInterFace::SM3MODE);
            QByteArray digest_pubKey(dataList.dequeue());
            if(digest_pubKey == scto.encrypt_HMAC(originKey, pubKey.data(), pubKey.size())){
                sm4_key = scto.newKey(SctoInterFace::SM4_KEY).first();
                scto.setMODE_SM2(pubKey,"");
                QByteArrayList cipher_keyList = scto.encrypt(sm4_key.data(),sm4_key.size());
                V(id);
                if(!cipher_keyList.isEmpty()){
                    socket->write("###" + cipher_keyList.first() + "###");
                    printf("verified successfully!\n");
                    emit established();
                    emit alive();
                    while(flag_stream_enable == false){
                        sleep(1);
                    }
                    timerID = startTimer(100);
                    return;
                }
                else{
                    printf("Fatal: failed to generate key!");
                    socket->write("wrong");
                    socket->disconnectFromHost();
                    return;
                }
            }
            else{
                V(id);
                printf("Fatal:pubkey was received,bu not verified correctly!");
                socket->write("wrong");
                sleep(10);
                socket->disconnectFromHost();
                return;
            }
        }
        else
            continue;
    }
        printf("Fatal verify failed!");
        emit quitMe();
    return;

}

void ServerThread::sendStream()
{
    if(!flag_stream_enable){
        printf("Check your serial.\n");
        socket->disconnectFromHost();
        sleep(1);
        return;
    }
    if(p_stream == nullptr){
        printf("Something Wrong...abort THREAD[%d]\n",numberId);
        socket->disconnectFromHost();
        sleep(1);
        return;
    }
    finishMutex.lock();
    if(finish_threadCount == 0){
        finish_threadCount ++;
        lockMutex.lock();
        w.lock();
        lockMutex.unlock();
        if(p_stream->size() != 0){
            p_data = p_stream->at(0);
            if(p_data.size() != 8)
                p_data = "";
        }
    }
    else{
        finish_threadCount ++;
    }
    w.unlock();
    finishMutex.unlock();
    if( !p_data.isEmpty() ){
        /*** read , encrypt and write ***/
        QByteArray data("#d#");
        packNumber = packNumber % 10;
        data += QString::number(packNumber,10);
        P(id);
        while(!scto.isOpen()){
            scto.open();
        }
        scto.setMODE_SM4(sm4_key, QByteArray(iv));
        foreach (QByteArray _data, scto.encrypt(p_data.data(),p_data.size())){
            data += _data;
        }
        V(id);
        if(numberId == 0 && file != nullptr)
        {
            qDebug() << QString("save %1 in /opt/cipherFile").arg(file->write(data));
            file->flush();
        }
        QByteArray displayData(data);
        if(DEBUGflag == true){
            displayData.chop(3);
            displayData.remove(0,4);
            qDebug() << displayData.toHex('-');
        }
        data += "%#%";
        qint64 len = socket->write(data);
        printf("\033[0;32mTHREAD(%d) \033[0m Number[%d],write size:%lld \n",numberId,packNumber,len);
        packNumber ++;
        /***       REW         ***/
    }
    wMutex.lock();
    isShareCompleted.wait(&wMutex);
    ++ buzy_Count;
    wMutex.unlock();
    wMutex.lock();
    -- buzy_Count ;
    wMutex.unlock();

    /* invoke */
    finishMutex.lock();
    if(finish_threadCount == 0){
        lockMutex.lock();
        lockMutex.unlock();
        p_data.clear();
    }
    finishMutex.unlock();
}

void ServerThread::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == timerID){
        sendStream();
        emit alive();
    }
}
