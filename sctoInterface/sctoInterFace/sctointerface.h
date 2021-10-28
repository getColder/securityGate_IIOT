#ifndef SCTOINTERFACE_H
#define SCTOINTERFACE_H

#include "sctoInterFace_global.h"
#include "sctooptions.h"
#include <QByteArray>
#include <iostream>


class SCTOINTERFACE_EXPORT SctoInterFace
{
public:
    SctoInterFace();
    virtual ~SctoInterFace();    
    enum SCTOMODE{NONEMODE = 0, SM2MODE = 12, SM3MODE = 13, SM4MODE = 14, SM4_DMAMODE = 24, SM2_KEY = 112, SM4_KEY = 114};


private:
    SctoOptions opt;

    SctoInterFace::SCTOMODE mode = NONEMODE;

public:
    void setMODE(SctoInterFace::SCTOMODE _mode);
    void setMODE_SM4(const QByteArray& key,const QByteArray& iv);
    void setMODE_SM2(const QByteArray& pubkey,const QByteArray& prikey);
    void setkey(const QByteArrayList& Key);
    const QByteArrayList newKey(SCTOMODE MODE);
    QByteArrayList encrypt(char* input, qint64 len);
    QByteArrayList decrypt(char* input, qint64 len);
    QByteArray encrypt_HMAC(QByteArray shareKey,char *input, qint64 len);
    inline bool open(){return opt.openScto();}
    inline bool close(){return opt.closeScto();}
    inline bool isOpen(){return opt.isOpen();}
};
#endif // SCTOINTERFACE_H
