#ifndef PATTERNS_H
#define PATTERNS_H
#include <QString>
#include <QRegExp>
#include <QQueue>
#include <QDebug>

class Patterns
{
public:
    Patterns();
    void filter(QByteArray &buf);
    QQueue<QString>& getCmdList();
    QQueue<QByteArray>& getDataList();
private:
    QRegExp cmdPattern;
    QQueue<QString> requestList;
    QQueue<QByteArray> dataList;
};

#endif // PATTERNS_H
