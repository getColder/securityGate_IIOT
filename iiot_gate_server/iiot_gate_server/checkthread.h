#ifndef CHECKTHREAD_H
#define CHECKTHREAD_H
#include <QThread>
#include <QMap>
#include <QTimerEvent>
#include <QMutex>
#include "serverthread.h"
#include <QTimerEvent>
#include <QQueue>

class checkThread :public QThread
{
    Q_OBJECT
public:
    checkThread();
    void run() override;
    inline void setThreadList(QMap<int, ServerThread *>* list){threadList = list;};

private:
    int numberID = 0;
    int aliveNumber = 0;
    int plusTimerID = -1;
    int zombie[200];
    int alive[200];
    int cleanTimerID = -1;
    QMap<int, ServerThread*> *threadList;
    void timerEvent(QTimerEvent *event) override;
    int findAvalibleNumberID();

public slots:
    void estNewConnection(ServerThread* newThread);
    void updateRefToBuf(QQueue<QByteArray> *bufList);
signals:
    void tEstablished();
    void stopT(int number);
};

#endif // CHECKTHREAD_H
