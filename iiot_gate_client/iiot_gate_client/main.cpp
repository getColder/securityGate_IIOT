#include <QCoreApplication>
#include <QRegExp>
#include "iiot_g_client.h"

QMutex queMutex;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Iiot_g_client client;
    return a.exec();
}
