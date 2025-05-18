#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QByteArray>
#include <QFile>

class MyTcpSocket
{
public:
    MyTcpSocket(QTcpSocket *);

public:
    QTcpSocket* get_tcpsocket();
    QByteArray read(qint64 max);
    QByteArray readAll();
    qint64 write(const QByteArray& data);

    void close();

public:
    const QString& get_name();            // 获取用户名
    void set_name(const QString& name);   // 设置用户名
    QFile* get_uploadFile();              // 获取上传文件
    bool get_uploading();                 //  获取上传状态
    void set_uploading(const bool& bl);   // 设置上传状态

    qint64 getUploadFileSize() const;                           // 获取上传文件大小
    void setUploadFileSize(const qint64 &uploadFileSize);       // 设置上传文件大小
    qint64 getUploadFileRecived() const;                        // 获取接收到的数据大小
    void setUploadFileRecived(const qint64 &uploadFileRecived); // 设置接收到的数据大小
    QString getUploadFileMd5() const;                           // 获取上传文件md5值
    void setUploadFileMd5(const QString& value);                // 设置上传文件md5值
    QString getCurrentPath() const;                             // 获取当前目录
    void setCurrentPath(const QString &value);                  // 设置当前目录
    QString getDownLoadFilePath() const;                        // 获取下载文件路径
    void setDownLoadFilePath(const QString &value);             // 设置下载文件路径

private:
    QTcpSocket *m_pTcpSocket;
    QString m_name;
    QFile m_uploadFile;
    qint64 m_uploadFileSize;
    qint64 m_uploadFileRecived;
    QString m_uploadFileMd5;
    bool m_uploading;
    QString m_currentPath;
    QString m_downLoadFilePath;
};

#endif // MYTCPSOCKET_H
