#ifndef SCTOOPTIONS_H
#define SCTOOPTIONS_H

#include <iostream>
#include <QDebug>
#include <QTime>
#include <QString>
#include <QByteArray>
#include <QByteArrayList>
#include <QMutex>
#include "sctoalgorithm.h"
#ifdef __cplusplus
extern "C"{
#endif
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#ifdef __cplusplus
}

#endif
#


class SctoOptions
{
public:
    SctoOptions();
    ~SctoOptions();
    bool openScto();
    bool closeScto();
    qint64 sizeOfList(QByteArrayList);
    qint64 outputMerge(const QByteArrayList&,char*& output);       // List<QByteArray>  --->  char*
    bool setKey(const QByteArrayList&);
    bool setIV_SM4(const QByteArray&);
    bool setSCTOMode(QString sctoMode);
    inline void loadKey_SM2(const QByteArray& pub,const QByteArray& pri){pub_Key_SM2 = pub; pri_Key_SM2 = pri;}
    inline void loadKey_IV_SM4(const QByteArray& key,const QByteArray& iv){key_SM4 = key;iv_SM4 = iv;}
    QByteArrayList encrypt(char* data, qint64 len);
    QByteArrayList decrypt(char* data, qint64 len);
    QByteArray encrypt_HMAC(QByteArray shareKey,char *input, qint64 len);
    const QByteArrayList geneKey_SM2() const;
    const QByteArrayList geneKey_SM4() const;
    bool isOpen();
private:
    SctoAlgorithm *sa;
    QByteArray pub_Key_SM2;
    QByteArray pri_Key_SM2;
    QByteArray key_SM4;
    QByteArray iv_SM4;
};


#endif // SCTOOPTIONS_H
