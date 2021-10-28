#include "sctooptions.h"
union semun{
    int val;
};
static int id = semget(123,1,IPC_CREAT|0644);
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


SctoOptions::SctoOptions():sa(NULL),pub_Key_SM2(),pri_Key_SM2(),key_SM4(),iv_SM4()
{
    if(id == -1){
        qDebug() << "-1 wrong";
        exit(1);
    }
    if(semctl(id, 0, GETVAL, 0) == 0){
        union semun su;
        su.val = 1;
        semctl(id, 0, SETVAL, su);
    }

}

SctoOptions::~SctoOptions()
{
    delete sa;
}



bool SctoOptions::openScto()
{
    P(id);
    SctoAlgorithm::fd = open("/dev/dma", O_RDWR);
    if( SctoAlgorithm::fd < 0 ){
        printf("open /dev/dma failed\n");
        qDebug() << SctoAlgorithm::fd << "false";
        V(id);
        return false;
    }
    SctoAlgorithm::openSet(true);
    V(id);
    return true;
}

bool SctoOptions::isOpen()
{
    if(SctoAlgorithm::fd > 0)
        return true;
    return false;
}

bool SctoOptions::closeScto()
{
    V(id);
    if(close(SctoAlgorithm::fd) != 0){
        V(id);
        return false;
    }
    SctoAlgorithm::openSet(false);
    V(id);
    return false;
}

bool SctoOptions::setSCTOMode(QString sctoMode)
{
    P(id);
    if(sa != NULL){
        V(id);
        delete sa;
    }
    if(sctoMode == "SM2"){
        sa = new SM2(pub_Key_SM2,pri_Key_SM2);
        V(id);
        return true;
    }
    else if(sctoMode == "SM3"){
        sa = new SM3();
        V(id);
        return true;
    }
    else if(sctoMode == "SM4"){
        sa = new SM4(key_SM4,iv_SM4);
        V(id);
        return true;
    }
    else if(sctoMode == "SM4_DMA"){
        sa = new SM4_DMA(key_SM4,iv_SM4);
        V(id);
        return true;
    }
    else{
        sa = NULL;
        V(id);
        return false;
    }

}

qint64 SctoOptions::sizeOfList(QByteArrayList list)
{
    qint64 size = 0;
    foreach(QByteArray data, list)
        size+= data.size();
    return size;
}

const QByteArrayList SctoOptions::geneKey_SM2() const
{
    P(id);
    QByteArrayList keyList = SctoAlgorithm::keyGet_SM2();
    V(id);
    return keyList;
}

const QByteArrayList SctoOptions::geneKey_SM4() const
{
    QByteArrayList newKeyList = QByteArrayList();
    QByteArray newKey;
    P(id);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    for(int i = 0; i < 16; i++){
        int ch = qrand()%255;
        newKey.push_back(ch);
    }
    newKeyList.append(newKey);
    V(id);
    return newKeyList;
}

qint64 SctoOptions::outputMerge(const QByteArrayList &outList, char*& outputArray)
{
    //**befor call to this, initialzed a arrary of SizeofList(outlist)*/
    if(outputArray == nullptr)
            return -1;
    qint64 pos = 0;
    foreach(QByteArray data, outList){
        for(int j = 0;j < data.size(); j++){
            outputArray[pos] = data.at(j);
            pos++;
        }
    }
    outputArray[++pos] = '\0';
    return pos;
}





bool SctoOptions::setKey(const QByteArrayList &_key)
{
    return sa->setKey(_key);
}

bool SctoOptions::setIV_SM4(const QByteArray &_iv)
{
    P(id);
    if(!QString(typeid (sa).name()).contains("SM4")){
        V(id);
        return false;
    }
    bool ok = dynamic_cast<SM4*>(sa)->setIV(_iv);
    V(id);
    return ok;
}

QByteArrayList SctoOptions::encrypt(char *data, qint64 len)
{
    QByteArrayList output;
    P(id);
    if(sa != NULL)
        output = sa->encrypt(data, len);
    V(id);
    return output;
}

QByteArrayList SctoOptions::decrypt(char *data, qint64 len)
{
    QByteArrayList output;
    P(id);
    if(sa != NULL)
        output = sa->decrypt(data, len);
    V(id);
    return output;
}

QByteArray SctoOptions::encrypt_HMAC(QByteArray shareKey,char *input, qint64 len)
{
    QByteArray hmac = dynamic_cast<SM3*>(sa)->encrypt_HMAC(shareKey,input,len);
    return hmac;
}
