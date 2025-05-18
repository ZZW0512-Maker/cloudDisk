#ifndef UPLOADFILETHREAD_H
#define UPLOADFILETHREAD_H

#include <QThread>
#include <QTcpSocket>

class UpLoadFileThread : public QThread
{
    Q_OBJECT

public:
    UpLoadFileThread(QString filePath);

protected:
    void run() override;

private:
    QString m_filePath;
};

#endif // UPLOADFILETHREAD_H
