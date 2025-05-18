#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class CloudClient; }
QT_END_NAMESPACE

class CloudClient : public QWidget
{
    Q_OBJECT

private:
    CloudClient(QWidget *parent = nullptr);
    CloudClient(const CloudClient&);
public:
    static CloudClient& get_instance();
    ~CloudClient();

public:
    void load_config();  // 加载配置文件，读取IP地址和端口号
    void init_network(); // 初始化网络连接

public:
    QTcpSocket* get_tcpSocket(); // 获取套接字
    QString& get_name(); // 获取用户名
    QString& get_current_dir(); // 获取当前目录
    QString set_current_dir(const QString& new_dir); // 设置当前目录

public slots:
    void connect_ok(); // 成功连接服务器槽函数
    void ready_read(); // 接收到服务器发来的消息

private slots:// 按键点击槽函数
    void on_pushButton_login_clicked();
    void on_pushButton_regist_clicked();
    void on_pushButton_cancal_clicked();

public:
    void user_regist_back(const QStringList& content); // 收到服务器返回注册结果处理函数
    void user_login_back(const QStringList& content); // 收到服务器返回登录结果处理函数
    void user_cancel_back(const QStringList& content); // 收到服务器返回注销结果处理函数
    void add_friend_back(const QStringList& content); // 收到服务器返回添加好友请求处理函数

private:
    Ui::CloudClient *ui;
    QTcpSocket *m_ptcpSocket;
    QString m_strIP;
    quint16 m_port;
    QString m_name;
    QString m_currentDir;/*存放用户当前所在目录*/
};
#endif // CLOUDCLIENT_H
