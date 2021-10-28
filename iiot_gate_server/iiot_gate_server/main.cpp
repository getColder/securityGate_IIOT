#include <QCoreApplication>
#include "iiot_g_server.h"
#include <QMutex>
#include <QWaitCondition>

char originKey[16] = {'a','k','e', 'y', 'h', 'm', 'a', 'c'};
char iv[17] = { 0xC7,0x2B,0x65,0x91,0xA0,0xD7,0xDE,0x8F,0x6B,0x40,0x72,0x33,0xAD,0x35,0x81,0xD6, 0x00};
QMutex mutex;
QMutex wMutex;
QMutex w;
QMutex newMutex;
QMutex threadListMutex;
QMutex finishMutex;
QMutex lockMutex;
QWaitCondition isShareCompleted;

union semun{
    int val;
};
int id = semget(1243,1,IPC_CREAT|0644);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    semun su;
    su.val = 1;
    semctl(id, 0, SETVAL, su);
    Iiot_g_server gateServer;

    return a.exec();
}
