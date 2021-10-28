#ifndef QTCPSERVER_RIMPL_H
#define QTCPSERVER_RIMPL_H
#include <QTcpServer>
#include <QObject>
#include "serverthread.h"

class QTcpServer_Rimpl: public QTcpServer
{
    Q_OBJECT
public:
    QTcpServer_Rimpl(QObject *parent = nullptr);
    QMap<int,ServerThread*> threadList;
    virtual ~QTcpServer_Rimpl();

protected:
    void incomingConnection(qintptr handle) override;

private:
    int wakeTimerID = -1;
    void timerEvent(QTimerEvent *event) override;

public slots:

signals:
    void toEstnewConnection(ServerThread* newThread);
};

#endif // QTCPSERVER_RIMPL_H
