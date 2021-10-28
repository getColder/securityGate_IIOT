#include "patterns.h"

Patterns::Patterns():cmdPattern("(?:#c#)(.*)(?:###)")
{
    cmdPattern.setMinimal(true);

}

void Patterns::filter(QByteArray &buf)
{
    int pos = 0;
    while((pos = cmdPattern.indexIn(buf,pos)) != -1){
        QString sub = cmdPattern.cap(0);
        requestList.enqueue(cmdPattern.cap(1));
        buf.replace(pos,sub.size(),"");
    }
    pos = 0;
    int count = 0;
    while(pos != -1){
        pos = buf.indexOf("#d#",count);
        if(pos == -1)
            break;
        int posEnd = buf.indexOf("##%",count);
        dataList += buf.mid(pos + 3,posEnd - (pos + 3));
        buf.remove(pos , posEnd + 2);
        count ++;
    }
}

QQueue<QString>& Patterns::getCmdList()
{
    return requestList;
}

QQueue<QByteArray>& Patterns::getDataList()
{
    return dataList;
}
