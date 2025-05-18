#ifndef CLOUDSERVE_H
#define CLOUDSERVE_H

#include <QWidget>
#include <QTcpServer>
#include <QVector>
#include <mytcpsocket.h>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class CloudServe; }
QT_END_NAMESPACE

class CloudServe : public QWidget
{
    Q_OBJECT

private:
    CloudServe(QWidget *parent = nullptr);
    CloudServe(const CloudServe&);
public:
    static CloudServe& get_instance();
    ~CloudServe();

public:
    void init_db();  // 初始化数据库连接
    void load_config(); // 加载配置文件
    void init_nerwork(); // 初始化网络连接

    // 网络相关槽函数
public slots:
    void new_connection(); // 新客户连接槽函数
    void dis_connected(); // 客户端下线
    void ready_read(); // 接收到客户端消息槽函数
public:// 处理请求函数
    void handle_user_regist(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户注册
    void handle_user_login(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户登录
    void handle_user_cancel(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户注销
    void handle_allonline_users(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户全部在线用户请求
    void handle_search_user(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理搜索用户请求
    void handle_add_friend(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理添加好友请求
    void handle_add_friend_agree(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户同意添加好友请求
    void handle_add_friend_refuse(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户拒绝添加好友请求
    void handle_online_friends(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户查询好友列表
    void handle_delete_friend(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户查询好友列表
    void handle_send_msg(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户发送消息
    void handle_create_dir(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户创建文件夹
    void handle_flush_file(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户刷新文件
    void handle_delete(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户删除文件夹
    void handle_enter_dir(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户进入文件夹
    void handle_rename_dir(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户进入文件夹
    void handle_upload_file(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户上传文件请求
    void handle_upload_file_content(MyTcpSocket *pmyTcpSocket,const QStringList& content,const QByteArray& buff); // 处理用户上传文件内容
    void handle_download_file(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户下载文件请求
    void handle_download_file_content(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户下载文件内容请求
    void handle_move_file(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户下载文件内容请求
    void handle_share_online_friends(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户获取分享在线好友列表
    void handle_share_file(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户分享文件请求
    void handle_share_file_agree(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户同意分享文件请求
    void handle_share_file_link(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理用户同意分享文件请求
    void handle_get_file_from_link(MyTcpSocket *pmyTcpSocket,const QStringList& content); // 处理从链接获取文件请求

public:
    MyTcpSocket* find_pmyTcpSocket(QTcpSocket *ptcpsocket);  // 根据QTcpSocket在容器中查找MyTcpSocket并返回MyTcpSocket
    bool delete_dir(QString path);                          // 删除目录
    bool copy_dir(QString srcDir,QString desDir);           // 拷贝目录

private:
    Ui::CloudServe *ui;
    QTcpServer *m_ptcpServe;
    QVector<MyTcpSocket *> m_pmyTcpSockt_vec;
    QString m_strIP;
    quint16 m_port;
};
#endif // CLOUDSERVE_H
