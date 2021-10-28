#include "sctoalgorithm.h"

int SctoAlgorithm::fd = -1;
int SctoAlgorithm::ret = 0;
typedef enum
{
    SM4_MODE_ECB = 0,   // ECB Mode
    SM4_MODE_CBC,   // CBC Mode
    SM4_MODE_CFB,   // CFB Mode
    SM4_MODE_OFB,   // OFB Mode
    SM4_MODE_CTR,   // CTR Mode
    SM4_CRYPTO_ENCRYPT = 0,
    SM4_CRYPTO_DECRYPT = 1,
    SM2_GET_PUBKEY = 10,
    SM2_GET_PRIKEY = 11
}SM4_MODE_E,SM4_CRYPTO_E,SM2_KEY_FLAG;

struct _st_sm2_key
{
    uint8_t priKey[32];
    uint8_t pubKey[65];
}SM2_KEY;

struct _st_sm2_encrypt
{
    uint8_t M[256];
    uint32_t MByteLen;
    uint8_t pubKey[65];
    uint8_t order;
    uint8_t C[512];
    uint32_t CByteLen;
}SM2_ENCRYPT;

struct _st_sm2_decrypt
{
    uint8_t C[512];
    uint32_t CByteLen;
    uint8_t priKey[32];
    uint8_t order;
    uint8_t M[256];
    uint32_t MByteLen;
}SM2_DECRYPT;

struct _st_cpu_init_
{
    SM4_MODE_E mode;
    SM4_CRYPTO_E crypto;
    uint8_t key[128];
    uint8_t iv[128];
}SM4_INIT;

struct _st_crypto_
{
    uint8_t in[128];
    uint8_t out[128];
    uint32_t byteLen;
}SM4_CRYPTO;


struct _st_dma_crypto_
{
    uint32_t in[128];
    uint32_t out[128];
    uint32_t byteLen;
}SM4_DMA_CRYPTO;

typedef struct _st_hash_context
{
    uint8_t first_update_flag;
    uint8_t finish_flag;
    uint8_t hash_buffer[64];
    uint32_t total[2];
}Context;

struct _st_hash_process
{
    Context hash_context;
    uint8_t input[128];
    uint32_t byteLen;
}SM3_HASH_PROCESS;

struct _st_hash_done
{
    Context context;
    uint8_t digest[32];
}SM3_HASH_DONE;

struct _st_hash
{
    uint8_t message[128];
    uint32_t byteLen;
    uint8_t digest[32];
}SM3_HASH;

static bool hasInit = false;
static bool isOpen = false;

#define SCTO_SM3_INIT              _IOR('k', 9,  Context *)
#define SCTO_SM3_PROCESS           _IOR('k', 10, struct _st_hash_process *)
#define SCTO_SM3_DONE              _IOR('k', 11, struct _st_hash_done *)
#define SCTO_SM3_HASH              _IOR('k', 12, struct _st_hash *)
#define SCTO_SM2_KEYGET            _IOR('k', 0, struct _st_sm2_key *)
#define SCTO_SM2_ENCRYPT           _IOR('k', 1, struct _st_sm2_encrypt *)
#define SCTO_SM2_DECRYPT           _IOR('k', 2, struct _st_sm2_decrypt *)
#define SCTO_SM4_INIT              _IOR('k', 20,  struct _st_cpu_init_ *)
#define SCTO_SM4_CRYPTO            _IOR('k', 21,  struct _st_crypto_ *)
#define SCTO_SM4_DMA_INIT          _IOR('k', 22,  struct _st_cpu_init_ *)
#define SCTO_SM4_DMA_CRYPTO        _IOR('k', 23,  struct _st_dma_crypto_ *)



SctoAlgorithm::~SctoAlgorithm()
{

}

QByteArrayList SctoAlgorithm::decrypt(const char* data,qint64 len)
{
    return inputAlignment(data, len, 32);
}

bool SctoAlgorithm::setKey(QByteArrayList)
{
    return false;
}

void SctoAlgorithm::openSet(bool _isOpen)
{
    isOpen = _isOpen;
}

void SctoAlgorithm::initSet(bool _hasInit)
{
    hasInit = _hasInit;
}

QByteArrayList SctoAlgorithm::inputAlignment(const char* data, qint64 length, int subLength)
{
    QByteArrayList dataArraysAligned = QByteArrayList();
    if (length <= 0 || subLength <= 0)
        return dataArraysAligned;
    while(1){
        if(length > subLength){
            QByteArray dataArray(data,subLength);
            dataArraysAligned.append(dataArray);
            data += subLength * sizeof (char);
            length -= subLength;
        }
        else {
            QByteArray arrayLast(subLength,'\0');
            arrayLast.replace(0, length, data, length);
            dataArraysAligned.append(arrayLast);
            QByteArray arrayLastGroupNumber(subLength,'0');
            arrayLastGroupNumber[0] = (char)length;
            dataArraysAligned.append(arrayLastGroupNumber);   // data count of the last group  to tail
            break;
        }
    }
    return dataArraysAligned;
}


QList<uint32_t> SctoAlgorithm::char4ToUint_32(const char *input, qint64 size)
{
    QList<uint32_t> data4List = QList<uint32_t>();
    qint64 realSize = size - (size % 4);
    QByteArrayList dataList = inputAlignment(input, realSize, 4);
    dataList.removeLast();
    foreach(QByteArray data, dataList){
        uint32_t data4 = (unsigned char)data.at(0);
        data4 <<= 8;
        data4 |= (unsigned char)data.at(1);
        data4 <<= 8;
        data4 |= (unsigned char)data.at(2);
        data4 <<= 8;
        data4 |= (unsigned char)data.at(3);
        data4List.append(data4);
    }
    return data4List;
}



/*** Class SM4 Implement ***/

SM4::SM4(QByteArray _key, QByteArray _IV):SctoAlgorithm(),key_SM4(_key),IV(_IV)
{
    bzero(&SM4_INIT, sizeof(SM4_INIT));
    if(key_SM4.count() != 16 || IV.count() != 16){
        hasInit = false;
        return;
    }
    for(int i = 0; i < 16; i++)
        SM4_INIT.key[i] = key_SM4.at(i);
    for(int i = 0; i < 16; i++)
        SM4_INIT.iv[i] = IV.at(i);
    hasInit = true;


}

bool SM4::setKey(QByteArrayList keyList)
{
    hasInit = false;
    if( keyList.size() != 1)
        return false;
    if(keyList.first().size() != 16)
        return false;
    for(int i = 0; i < 16; i++)
        SM4_INIT.key[i] = keyList.first().at(i);
    hasInit = true;
    return true;
}

bool SM4::setIV(QByteArray _iv)
{
    hasInit = false;
    if(_iv.size() != 16)
        return false;
    for(int i = 0; i < 16; i++)
        SM4_INIT.iv[i] = _iv.at(i);
    hasInit = true;
    return true;
}



QByteArrayList SM4::encrypt(const char* inputArray, qint64 inLength)
{
    QByteArrayList _outList = QByteArrayList();
    if(!(isOpen&&hasInit) || inLength <= 0)
        return _outList;
    QByteArrayList _dataList = inputAlignment(inputArray,inLength,16);
    SM4_INIT.crypto = SM4_CRYPTO_ENCRYPT;
    SM4_INIT.mode = SM4_MODE_CBC;
    for(int i = 0; i < _dataList.size(); i++ ) {
        /*init scto*/
        bzero(&SM4_CRYPTO, sizeof(SM4_CRYPTO));
        SM4_CRYPTO.byteLen = 16;
        ret = ioctl(fd, SCTO_SM4_INIT, &SM4_INIT);
        if( ret != 0){
            printf("SCTO_SM4_INIT failed with ret:%d\n",ret);
            return _outList;
        }
        /*write group */
        for(int k = 0; k < 16; k++)
            SM4_CRYPTO.in[k] = _dataList.at(i).at(k);
        /*encrypt*/
        ret = ioctl(fd, SCTO_SM4_CRYPTO, &SM4_CRYPTO);
        if( ret != 0){
            printf("SCTO_SM4_Encryption failed with ret:%d\n",ret);
            return _outList;
        }
        /*output*/
        _outList.append(QByteArray((char*)SM4_CRYPTO.out,16));
    }
    return _outList;
}

QByteArrayList SM4::decrypt(const char* inputArray, qint64 inLength)
{
    QByteArrayList _outList;
    if(!(isOpen&&hasInit) || inLength <= 0)
        return _outList;
    QByteArrayList _dataList;
    for(int i = 0;i < inLength/16; i++){
        _dataList.append(QByteArray(inputArray,16));
        inputArray += 16;
    }
    if(_dataList.size() < 2){
        return _outList;
    }
    SM4_INIT.crypto = SM4_CRYPTO_DECRYPT;
    SM4_INIT.mode = SM4_MODE_CBC;

/*** decrypt data groups ***/
    for(int i = 0; i < _dataList.size() - 2 ; i++ ) {
        /*init scto*/
        bzero(&SM4_CRYPTO, sizeof(SM4_CRYPTO));
        SM4_CRYPTO.byteLen = 16;
        ret = ioctl(fd, SCTO_SM4_INIT, &SM4_INIT);
        if( ret != 0){
            printf("SCTO_SM4_INIT failed with ret:%d\n",ret);
            return _outList;
        }
        /*write group */
        for(int k = 0; k < 16; k++)
            SM4_CRYPTO.in[k] = _dataList.at(i).at(k);
        /*decrypt*/
        ret = ioctl(fd, SCTO_SM4_CRYPTO, &SM4_CRYPTO);
        if( ret != 0){
            printf("SCTO_SM4_Encryption failed with ret:%d\n",ret);
            return _outList;
        }
        /*output except last group*/
        _outList.append(QByteArray((char*)SM4_CRYPTO.out,SM4_CRYPTO.byteLen));
    }
/*** get last group data number ***/
    bzero(&SM4_CRYPTO, sizeof(SM4_CRYPTO));
    SM4_CRYPTO.byteLen = 16;
    ret = ioctl(fd, SCTO_SM4_INIT, &SM4_INIT);
    if( ret != 0){
        printf("SCTO_SM4_INIT failed with ret:%d\n",ret);
        return _outList;
    }
    for(int k = 0; k < 16; k++)
        SM4_CRYPTO.in[k] = _dataList.last().at(k);
    ret = ioctl(fd, SCTO_SM4_CRYPTO, &SM4_CRYPTO);
    if( ret != 0){
        printf("SCTO_SM4_Encryption failed with ret:%d\n",ret);
        return _outList;
    }
    int lastGroupNumber = (int)QByteArray((char*)SM4_CRYPTO.out,16).at(0);
/*last group to decrypt and kill zero*/
    bzero(&SM4_CRYPTO, sizeof(SM4_CRYPTO));
    SM4_CRYPTO.byteLen = 16;
    ret = ioctl(fd, SCTO_SM4_INIT, &SM4_INIT);
    if( ret != 0){
        printf("SCTO_SM4_INIT failed with ret:%d\n",ret);
        return _outList;
    }
    for(int k = 0; k < 16; k++)
        SM4_CRYPTO.in[k] = _dataList.at(_dataList.count() - 2).at(k);
    ret = ioctl(fd, SCTO_SM4_CRYPTO, &SM4_CRYPTO);
    if( ret != 0){
        printf("SCTO_SM4_Encryption failed with ret:%d\n",ret);
        return _outList;
    }
    QByteArray lastGroupData(QByteArray((char*)SM4_CRYPTO.out,lastGroupNumber));
    _outList.append(lastGroupData);

    return _outList;
}

SM4_DMA::SM4_DMA(QByteArray _key, QByteArray _IV):SctoAlgorithm(),key_SM4(_key),IV(_IV)
{
    bzero(&SM4_INIT, sizeof(SM4_INIT));
    if(key_SM4.count() != 16 || IV.count() != 16){
        printf("Wrong Key or IV");
        hasInit = false;
        return;
    }
    for(int i = 0; i < 16; i++)
        SM4_INIT.key[i] = key_SM4.at(i);
    for(int i = 0; i < 16; i++)
        SM4_INIT.iv[i] = IV.at(i);
    hasInit = true;
}

QByteArrayList SM4_DMA::encrypt(const char* inputArray, qint64 inLength)
{
    QByteArrayList _outList = QByteArrayList();
    if(!(isOpen&&hasInit) || inLength <= 0)
        return _outList;
    QList<uint32_t> _dataList = char4ToUint_32(inputArray,inLength);
    SM4_INIT.crypto = SM4_CRYPTO_ENCRYPT;
    SM4_INIT.mode = SM4_MODE_CBC;
    /*init scto*/
    bzero(&SM4_DMA_CRYPTO, sizeof(SM4_DMA_CRYPTO));
    SM4_DMA_CRYPTO.byteLen = _dataList.size();
    ret = ioctl(fd, SCTO_SM4_DMA_INIT, &SM4_INIT);
    if( ret != 0){
        printf("SCTO_SM4_INIT failed with ret:%d\n",ret);
        return _outList;
    }
    int i = 0;
    /*write group */
    foreach(uint32_t data, _dataList) {
        SM4_DMA_CRYPTO.in[i] = data;
        i++;
    }
    /*encrypt*/
    ret = ioctl(fd, SCTO_SM4_DMA_CRYPTO, &SM4_DMA_CRYPTO);
    if( ret != 0){
        printf("SCTO_SM4_Encryption failed with ret:%d\n",ret);
        return _outList;
    }
    /*output*/
    for(int i = 0;i < 128; i++){
        qDebug() << QString::number(SM4_DMA_CRYPTO.out[i],16);
    }
    return _outList;

}

QByteArrayList SM4_DMA::decrypt(const char* inputArray, qint64 inLength)
{
    QByteArrayList _outList = QByteArrayList();
    if(!(isOpen&&hasInit) || inLength <= 0)
        return _outList;
    QList<uint32_t> _dataList = char4ToUint_32(inputArray,inLength);
    SM4_INIT.crypto = SM4_CRYPTO_DECRYPT;
    SM4_INIT.mode = SM4_MODE_CBC;
    /*init scto*/
    bzero(&SM4_DMA_CRYPTO, sizeof(SM4_DMA_CRYPTO));
    SM4_DMA_CRYPTO.byteLen = _dataList.size();
    ret = ioctl(fd, SCTO_SM4_DMA_INIT, &SM4_INIT);
    if( ret != 0){
        printf("SCTO_SM4_INIT failed with ret:%d\n",ret);
        return _outList;
    }
    int i = 0;
    /*write group */
    foreach(uint32_t data, _dataList) {
        SM4_DMA_CRYPTO.in[i] = data;
        i++;
    }
    /*encrypt*/
    ret = ioctl(fd, SCTO_SM4_DMA_CRYPTO, &SM4_DMA_CRYPTO);
    if( ret != 0){
        printf("SCTO_SM4_Encryption failed with ret:%d\n",ret);
        return _outList;
    }
    /*output*/
    for(int i = 0;i < 128; i++){
        qDebug() << QString::number(SM4_DMA_CRYPTO.out[i],16);
    }
    return _outList;
}

const QByteArrayList SctoAlgorithm::keyGet_SM2()
{
    bzero(&SM2_KEY,sizeof(SM2_KEY));
    QByteArrayList keyList = QByteArrayList();
    ret = ioctl(fd, SCTO_SM2_KEYGET, &SM2_KEY);
    if( ret != 0 ){
        printf("SCTO_SM2_KEYGET failed with ret:%d\n",ret);
        close(fd);
        return keyList;
    }
    keyList += QByteArray((char*)SM2_KEY.pubKey,65);
    keyList += QByteArray((char*)SM2_KEY.priKey,32);
    bzero(&SM2_KEY,sizeof(SM2_KEY));
    return keyList;
}

SM2::SM2(QByteArray _pubKey, QByteArray _priKey):SctoAlgorithm(),pubKey_SM2(_pubKey),priKey_SM2(_priKey)
{
    hasInit = true;
}

bool SM2::setKey(QByteArrayList keyList)
{
    hasInit = false;
    if( keyList.size() != 2)
        return false;
    if(keyList.first().size() != 65)
        return false;
    if(keyList.last().size() != 32)
        return false;
    for(int i = 0; i < 65; i++)
        pubKey_SM2[i] = keyList.first().at(i);
    for(int i = 0; i < 32; i++)
        pubKey_SM2[i] = keyList.last().at(i);
    hasInit = true;
    return true;
}

QByteArrayList SM2::encrypt(const char* inputArray,qint64 inLength)
{
    QByteArrayList _outList = QByteArrayList();
    if(!(isOpen&&hasInit) || inLength <= 0)
        return _outList;
    if(pubKey_SM2.size() != 65)
        return _outList;
    QByteArrayList _dataList = inputAlignment(inputArray, inLength, 256);
    int len = inLength;
    _dataList.removeLast();
    foreach(QByteArray data, _dataList){
        if(len > 256)
            len -= 256;
        bzero(&SM2_ENCRYPT,sizeof(SM2_ENCRYPT));
        for(int i = 0; i < len; i++)
            SM2_ENCRYPT.M[i] = data.at(i);
        SM2_ENCRYPT.MByteLen = len;
        for(int i = 0; i < 65; i++)
            SM2_ENCRYPT.pubKey[i] = pubKey_SM2.at(i);
        SM2_ENCRYPT.order = 0;
        ret = ioctl(fd, SCTO_SM2_ENCRYPT, &SM2_ENCRYPT);
        if( ret != 0 ){
            printf("SCTO_SM2_ENCRYPT failed with ret:%d\n",ret);
            close(fd);
            return _outList;
        }
        _outList += QByteArray((char*)SM2_ENCRYPT.C,SM2_ENCRYPT.CByteLen);
    }
    return  _outList;
}

QByteArrayList SM2::decrypt(const char *inputArray, qint64 inLength)
{
    QByteArrayList _outList = QByteArrayList();
    if(!(isOpen&&hasInit) || inLength <= 0)
        return _outList;
    if(priKey_SM2.size() != 32)
        return _outList;
    QByteArrayList _dataList = inputAlignment(inputArray, inLength, 512);
    int len = inLength;
    _dataList.removeLast();
    foreach(QByteArray data, _dataList){
        if(len > 512)
            len -= 512;
        bzero(&SM2_DECRYPT,sizeof(SM2_DECRYPT));
        for(int i = 0; i < len; i++)
            SM2_DECRYPT.C[i] = data.at(i);
        SM2_DECRYPT.CByteLen = len;
        for(int i = 0; i < 32; i++)
            SM2_DECRYPT.priKey[i] = priKey_SM2.at(i);
        SM2_DECRYPT.order = 0;
        ret = ioctl(fd, SCTO_SM2_DECRYPT, &SM2_DECRYPT);
        if( ret != 0 ){
            printf("SCTO_SM2_DECRYPT failed with ret:%d\n",ret);
            close(fd);
            return _outList;
        }
        _outList += QByteArray((char*)SM2_DECRYPT.M,SM2_DECRYPT.MByteLen);
    }
    return  _outList;
}

SM3::SM3()
{
    hasInit = true;
}

QByteArrayList SM3::encrypt(const char *inputArray, qint64 inLength)
{
    Context SM3_Context;
    QByteArrayList _outList = QByteArrayList();
    if(!(isOpen&&hasInit) || inLength <= 0)
        return _outList;
    ret = ioctl(fd, SCTO_SM3_INIT, &SM3_Context);
    if( ret != 0){
        printf("SCTO_SM3_INIT failed with ret:%d\n", ret);
        close(fd);
        return _outList;
    }
    bzero(&SM3_HASH_PROCESS,sizeof(SM3_HASH_PROCESS));
    SM3_HASH_PROCESS.hash_context = SM3_Context;
    QByteArrayList _dataList = inputAlignment(inputArray,inLength, 32);
    _dataList.removeLast();
    foreach(QByteArray data, _dataList){
                for(int i = 0; i < data.size();  i++)
                    SM3_HASH_PROCESS.input[i] = data.at(i);
                SM3_HASH_PROCESS.byteLen = data.size();
                ret = ioctl(fd, SCTO_SM3_PROCESS, &SM3_HASH_PROCESS);
                if( ret != 0 ){
                    printf("SCTO_SM3_PROCESS failed with ret:%d\n", ret);
                    close(fd);
                    return _outList;
                }
                bzero(&SM3_HASH_DONE,sizeof(SM3_HASH_DONE));
                SM3_HASH_DONE.context = SM3_HASH_PROCESS.hash_context;
                ret = ioctl(fd, SCTO_SM3_DONE, &SM3_HASH_DONE);
                if(ret != 0){
                    printf("SCTO_SM3_DONE failed with ret:%d\n",ret);
                    close(fd);
                    return _outList;
                }

                bzero(&SM3_HASH,sizeof(SM3_HASH));
                memcpy(SM3_HASH.message, data.data(), sizeof(SM3_HASH.message));
                SM3_HASH.byteLen = data.size();
                ret = ioctl(fd, SCTO_SM3_HASH, &SM3_HASH);
                if( ret != 0){
                    printf("SCTO_SM3_HASH failed with ret:%d\n", ret);
                    close(fd);
                    return _outList;
        }
        _outList.append(QByteArray((char*)SM3_HASH.digest,32));
    }
    return _outList;
}

QByteArray SM3::encrypt_HMAC(QByteArray _shareKey, const char *inputArray, qint64 inLength)
{
    QByteArray iPad('\x36',32);
    QByteArray OPad('\x5c',32);
    QByteArray shareKEY(32,'0');
    shareKEY.replace(0,_shareKey.size(),_shareKey.data());
    QByteArray ipadKey;
    for(int i = 0;i < 32; i++)
        ipadKey += (iPad.at(i) ^ shareKEY.at(i));
    QByteArray opadKey;
    for(int i = 0;i < 32; i++)
        opadKey += (OPad.at(i) ^ shareKEY.at(i));
    QByteArray HMAC1_temp;
    QByteArray INPUT(inputArray,inLength);
    int count = 0;
    while(ipadKey.size() > count || INPUT.size() > count){
        if(ipadKey.size() > count)
            HMAC1_temp.append(ipadKey.at(count));
        if(INPUT.size() > count)
            HMAC1_temp.append(INPUT.at(count));
        count++;
    }
    QByteArray HMAC_1;
    foreach(QByteArray unit, SM3::encrypt(HMAC1_temp.data(), HMAC1_temp.size()))
        HMAC_1.append(unit);


    QByteArray HMAC2_temp;
    count = 0;
    while(opadKey.size() > count || HMAC_1.size() > count){
        if(opadKey.size() > count)
            HMAC2_temp.append(opadKey.at(count));
        if(HMAC_1.size() > count)
            HMAC2_temp.append(HMAC_1.at(count));
        count++;
    }

    QByteArray HMAC_2;
    foreach(QByteArray unit, SM3::encrypt(HMAC2_temp.data(), HMAC2_temp.size()))
        HMAC_2.append(unit);
    return  HMAC_2;
}

