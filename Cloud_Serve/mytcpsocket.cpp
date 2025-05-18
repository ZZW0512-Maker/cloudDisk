#include "mytcpsocket.h"

MyTcpSocket::MyTcpSocket(QTcpSocket *tcpsocket)
    : m_pTcpSocket(tcpsocket)
    , m_name()
    , m_uploadFile()
    , m_uploadFileSize()
    , m_uploadFileRecived()
    , m_uploadFileMd5()
    , m_uploading(false)
    , m_currentPath()
    , m_downLoadFilePath()
{

}

QTcpSocket *MyTcpSocket::get_tcpsocket()
{
    return m_pTcpSocket;
}

QByteArray MyTcpSocket::read(qint64 maxlen)
{
    return m_pTcpSocket->read(maxlen);
}

QByteArray MyTcpSocket::readAll()
{
    return m_pTcpSocket->readAll();
}

qint64 MyTcpSocket::write(const QByteArray &data)
{
    return m_pTcpSocket->write(data);
}

void MyTcpSocket::close()
{
    return m_pTcpSocket->close();
}

const QString &MyTcpSocket::get_name()
{
    return m_name;
}

void MyTcpSocket::set_name(const QString &name)
{
    m_name = name;
    return;
}

QFile* MyTcpSocket::get_uploadFile()
{
    return &m_uploadFile;
}

bool MyTcpSocket::get_uploading()
{
    return m_uploading;
}

void MyTcpSocket::set_uploading(const bool &bl)
{
    m_uploading = bl;
    return;
}

qint64 MyTcpSocket::getUploadFileSize() const
{
    return m_uploadFileSize;
}

void MyTcpSocket::setUploadFileSize(const qint64 &uploadFileSize)
{
    m_uploadFileSize = uploadFileSize;
}

qint64 MyTcpSocket::getUploadFileRecived() const
{
    return m_uploadFileRecived;
}

void MyTcpSocket::setUploadFileRecived(const qint64 &uploadFileRecived)
{
    m_uploadFileRecived = uploadFileRecived;
}

QString MyTcpSocket::getUploadFileMd5() const
{
    return m_uploadFileMd5;
}

void MyTcpSocket::setUploadFileMd5(const QString& value)
{
    m_uploadFileMd5 = value;
}

QString MyTcpSocket::getCurrentPath() const
{
    return m_currentPath;
}

void MyTcpSocket::setCurrentPath(const QString &value)
{
    m_currentPath = value;
}

QString MyTcpSocket::getDownLoadFilePath() const
{
    return m_downLoadFilePath;
}

void MyTcpSocket::setDownLoadFilePath(const QString &value)
{
    m_downLoadFilePath = value;
}
