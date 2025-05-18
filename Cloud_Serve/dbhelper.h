#ifndef DBHELPER_H
#define DBHELPER_H

#include <QSqlDatabase>
#include <QStringList>

class DbHelper
{
private:
    DbHelper();
    ~DbHelper();
    DbHelper(const DbHelper&);

public:
    bool init_db(); // 初始化数据库连接

public:
    static DbHelper& get_instance();

    QString user_regist(const QString& name,const QString& pwd);//处理用户注册
    QString user_login(const QString& name,const QString& pwd);//处理用户登录
    bool user_offline(const QString& name);//处理用户退出登录
    QString user_cancel(const QString& name,const QString& pwd);//处理用户注销

    QStringList get_allonline_users(); // 获取全部在线用户
    QStringList get_online_friends(const QString& name); // 获取在线好友列表
    QString get_search_user(const QString& name);//处理搜索用户

    int handle_add_friend(const QString& src_name,const QString& per_name); // 处理用户添加好友请求
    bool handle_add_friend_agree(const QString& src_name,const QString& per_name); // 处理用户同意添加好友
    bool handle_delete_friend(const QString& src_name,const QString& per_name); // 处理用户删除好友

    bool handle_save_fileInfo(const QString& filepath,const QString& md5);          // 保存文件路径和md5值
    QString handle_file_isExited(const QString& md5);          // 查询数据库是否有该文件
    bool handle_create_share_link(QString filePath, QString link, QString createTime, int timeout); //在数据库中添加分享连接信息
    QString get_file_by_link(const QString &link, char* filePath);                       // 根据链接查找文件

public:
    QString generateMd5Hash(const QString &pwd); // 将密码转换成哈希值

private:
    QSqlDatabase m_db;
    QString m_hostName;
    int m_port;
    QString m_userName;
    QString m_userPassword;
    QString m_databaseName;
};

#endif // DBHELPER_H
