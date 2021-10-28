#include "sctointerface.h"




SctoInterFace::SctoInterFace():opt()
{
}



SctoInterFace::~SctoInterFace()
{
    printf("scto instance destroyed\n");
}

void SctoInterFace::setMODE(SctoInterFace::SCTOMODE _mode)
{
    switch (_mode) {
    case 0:
        break;
    case 12:
        mode = SM2MODE;
        opt.setSCTOMode("SM2");
        break;
    case 13:
        mode = SM3MODE;
        opt.setSCTOMode("SM3");
        break;
    case 14:
        mode = SM4MODE;
        opt.setSCTOMode("SM4");
        break;
    case 24:
        mode = SM4_DMAMODE;
        opt.setSCTOMode("SM4_DMA");
        break;
    default:
        mode = NONEMODE;
        printf("set mode failed!");
    }
}

void SctoInterFace::setMODE_SM2(const QByteArray &_pubKey, const QByteArray &_prikey)
{
    opt.loadKey_SM2(_pubKey,_prikey);
    opt.setSCTOMode("SM2");
}

void SctoInterFace::setMODE_SM4(const QByteArray &_key, const QByteArray &_iv)
{
    opt.loadKey_IV_SM4(_key,_iv);
    opt.setSCTOMode("SM4");
}


void SctoInterFace::setkey(const QByteArrayList &keyList)
{
    if( opt.setKey(keyList) )
        qDebug() << "keys have been set.";
    else
        qDebug() << "failed! check your keys";

}

const QByteArrayList SctoInterFace::newKey(SCTOMODE flag)
{
    switch (flag) {
    case 112:{
        QByteArrayList keyList = QByteArrayList(opt.geneKey_SM2());
        return keyList;
    }
        break;
    case 114:
        return opt.geneKey_SM4();
        break;
    default:
        qDebug() << "failed; Wrong choose flag.";
        break;
    }
    return QByteArrayList();
}

QByteArrayList SctoInterFace::encrypt(char *input, qint64 len)
{
    return opt.encrypt(input,len);
}

QByteArray SctoInterFace::encrypt_HMAC(QByteArray shareKey,char *input, qint64 len)
{
    if(mode == SM3MODE)
        return opt.encrypt_HMAC(shareKey,input,len);
    else{
        printf("Wrong Scto MODE\n");
        return QByteArray();
    }

}

QByteArrayList SctoInterFace::decrypt(char *input, qint64 len)
{
    return opt.decrypt(input,len);
}
