#ifndef SCTOALGORITHM_H
#define SCTOALGORITHM_H
#include <QByteArray>
#include <QByteArrayList>
#include <QDebug>

#ifdef __cplusplus
extern "C"{
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
}
#endif


class SctoAlgorithm
{
public:
    SctoAlgorithm(){};
    virtual ~SctoAlgorithm();
    virtual QByteArrayList encrypt(const char*,qint64) = 0;
    virtual QByteArrayList decrypt(const char*,qint64);
    virtual bool setKey(QByteArrayList);
    QByteArrayList inputAlignment(const char*,qint64,int);    // char*  --->  QList<QByteArray>
    QList<uint32_t> char4ToUint_32(const char*, qint64);    // char x 4 --->   uInt_32 x 1
    static const QByteArrayList keyGet_SM2();
    static void openSet(bool _isOPen);
    static void initSet(bool _hasInit);
    static int fd;
    static int ret;

};

/*** Class SM4 ***/
class SM4 : public SctoAlgorithm
{
public:
    SM4(QByteArray _key, QByteArray _IV);
    virtual QByteArrayList encrypt(const char* inputArray,qint64 inLength) override;
    virtual QByteArrayList decrypt(const char* inputArray,qint64 inLength) override;
    virtual bool setKey(QByteArrayList) override;
    bool setIV(QByteArray);
private:
    QByteArray key_SM4;
    QByteArray IV;

};
//Algorithm of SM4

/*** Class SM4_DMA ***/
class SM4_DMA : public SctoAlgorithm
{
public:
    SM4_DMA(QByteArray _key, QByteArray _IV);
    virtual QByteArrayList encrypt(const char* inputArray,qint64 inLength) override;
    virtual QByteArrayList decrypt(const char* inputArray,qint64 inLength) override;
private:
    QByteArray key_SM4;
    QByteArray IV;

};
//Algorithm of SM4_DMA


/*** Class SM2 ***/
class SM2 : public SctoAlgorithm
{
public:
    SM2(QByteArray _pubKey, QByteArray _priKey);
    virtual QByteArrayList encrypt(const char* inputArray,qint64 inLength) override;
    virtual QByteArrayList decrypt(const char* inputArray,qint64 inLength) override;
    virtual bool setKey(QByteArrayList) override;
private:
    QByteArray pubKey_SM2;
    QByteArray priKey_SM2;

};
//Algorithm of SM2

/*** Class SM3 ***/
class SM3 : public SctoAlgorithm
{
public:
    SM3();
    virtual QByteArrayList encrypt(const char* inputArray,qint64 inLength) override;
    QByteArray encrypt_HMAC(QByteArray shareKEY, const char *inputArray, qint64 inLength);
private:

};
//Algorithm of SM3

#endif // SCTOALGORITHM_H
