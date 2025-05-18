#include "uploadfilethread.h"
#include "cloudclient.h"
#include <QMessageBox>

UpLoadFileThread::UpLoadFileThread(QString filePath)
    :  m_filePath(filePath)
{
    qDebug() << m_filePath;
}

void UpLoadFileThread::run()
{
    QFile file(m_filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        char buff[4096];
        qint64 ret = 0;
        /*循环读取，发送*/
        while(true)
        {
            memset(buff,0,sizeof(buff));
            ret = file.read(buff,4096);
            if(ret > 0)
            {
                CloudClient::get_instance().get_tcpSocket()->write((char*)buff,ret);
            }
            else if(ret == 0)//读取完成
            {
                break;
            }
            else
            {
                qDebug() << "文件读取错误";
            }
        }
        file.close();
    }
    else
    {
        qDebug() << "文件打开失败";
    }
    return;
}
