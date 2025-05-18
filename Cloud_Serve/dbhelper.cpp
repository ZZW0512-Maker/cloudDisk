#include "dbhelper.h"
#include "pack.h"
#include <QFile>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QVariant>
#include <QDebug>
#include <QDateTime>

DbHelper::DbHelper() : m_db(QSqlDatabase::addDatabase("QODBC"))
{

}

DbHelper::~DbHelper()
{
    m_db.close();
}

DbHelper &DbHelper::get_instance()
{
    static DbHelper instance;
    return instance;
}

bool DbHelper::init_db()
{
    QFile file(":/db.cfg");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "数据库配置文件读取失败！";
        return false;
    }
    QTextStream in(&file);

    // 按行读取配置文件
    m_hostName = in.readLine().trimmed();
    m_port = in.readLine().toInt();
    m_userName = in.readLine().trimmed();
    m_userPassword = in.readLine().trimmed();
    m_databaseName = in.readLine().trimmed();

    m_db.setHostName(m_hostName);
    m_db.setPort(m_port);
    m_db.setUserName(m_userName);
    m_db.setPassword(m_userPassword);
    m_db.setDatabaseName(m_databaseName);

    if(!m_db.open())
    {
        return false;
    }
    return true;
}

/*
 * 参数：name：用户名; passwd：密码
 * 处理用户注册逻辑
 * 0.注册成功;
 * 1.重复注册
 * 2.数据库异常错误
*/
QString DbHelper::user_regist(const QString &name, const QString &pwd)
{
    QSqlQuery query;
    QString strsql = QString("select * from user where name = '%1';").arg(name);
    if(!query.exec(strsql))
    {
        qDebug() << query.lastError();
        return MYSQL_ERROR;
    }
    else
    {
        if(query.next())
        {
            return USER_REGIST_FAILD_1;
        }
    }
    QString hash_pwd = generateMd5Hash(pwd);
    strsql = QString("insert into user(name,pwd) values('%1','%2');").arg(name).arg(hash_pwd);
    if(query.exec(strsql))
    {
        return USER_REGIST_OK;
    }
    qDebug() << query.lastError();
    return MYSQL_ERROR;
}

/*
 * 参数：name：用户名; passwd：密码
 * 处理用户登录逻辑
 * 0.登录成功;
 * 1.用户名或密码错误
 * 2.数据库异常错误
 * 3.重复登录
*/
QString DbHelper::user_login(const QString &name, const QString &pwd)
{
    QSqlQuery query;
    QString strsql = QString("select * from user where name = '%1';").arg(name);
    if(!query.exec(strsql))
    {
        qDebug() << query.lastError();
        return MYSQL_ERROR;
    }
    else
    {
        if(query.next()) // 如果有该用户
        {
            QString hash_pwd = generateMd5Hash(pwd);
            if(query.value("pwd").toString() == hash_pwd)
            {
                if(query.value("online").toInt() == 1) // 如果用户在线状态为“在线”
                {
                    return USER_LOGIN_FAILD_3; // 重复登录
                }
                else
                {
                    strsql = QString("update user set online = 1 where name = '%1';").arg(name); // 将在线状态改为在线
                    if(query.exec(strsql))
                    {
                        return USER_LOGIN_OK;
                    }
                    else
                    {
                        qDebug() << query.lastError();
                        return MYSQL_ERROR;
                    }
                }
            }
            else
            {
                return USER_LOGIN_FAILD_1; // 密码错误
            }
        }
        else
        {
            return USER_LOGIN_FAILD_1; // 用户名不存在
        }
    }
}

bool DbHelper::user_offline(const QString &name)
{
    QSqlQuery query;
    QString strsql = QString("update user set online = 0 where name = '%1';").arg(name); // 将在线状态改为离线
    if(query.exec(strsql))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
 * 参数：name：用户名; passwd：密码
 * 处理用户注销逻辑
 * 0.注销成功;
 * 1.用户名或密码错误
 * 2.数据库异常错误
*/
QString DbHelper::user_cancel(const QString &name, const QString &pwd)
{
    QSqlQuery query;
    QString strsql = QString("select * from user where name = '%1';").arg(name);
    if(!query.exec(strsql))
    {
        qDebug() << query.lastError();
        return MYSQL_ERROR;
    }
    else
    {
        if(query.next()) // 如果数据库中有该用户
        {
            QString hash_pwd = generateMd5Hash(pwd);
            if(query.value("pwd").toString() == hash_pwd) // 如果密码正确
            {
                strsql = QString("delete from friend where (id in (select id from user where name='%1')) "
                                 "or (friend_id in (select id from user where name='%2'))").arg(name).arg(name);
                if(!query.exec(strsql))
                {
                    return MYSQL_ERROR;
                }
                strsql = QString("delete from user where name = '%1' and pwd = '%2';").arg(name).arg(hash_pwd);
                if(query.exec(strsql))
                {
                    return USER_CANCEL_OK;
                }
                return MYSQL_ERROR;
            }
            else // 密码错误
            {
                return USER_CANCEL_FAILD_1;
            }
        }
        else // 数据库中没有该用户
        {
            return USER_CANCEL_FAILD_1;
        }
    }
}

QStringList DbHelper::get_allonline_users()
{
    QSqlQuery query;
    QStringList users;
    QString strsql = QString("select name from user where online = 1;");
    if(query.exec(strsql))
    {
        while(query.next())
        {
            users.append(query.value("name").toString());
        }
    }
    return users;
}

QStringList DbHelper::get_online_friends(const QString &name)
{
    QSqlQuery query;
    QStringList online_friends;
    // 1.请求者作为id者
    QString strsql = QString("select name from user where id in (select friend_id from friend where "
                             "id = (select id from user where name='%1')) and online=1;").arg(name);
    if(query.exec(strsql))
    {
        while(query.next())
        {
            online_friends.append(query.value("name").toString());
        }
    }
    // 2. 请求者作为friend_id者
    strsql = QString("select name from user where id in (select id from friend where "
                     "friend_id = (select id from user where name='%1')) and online=1;").arg(name);
    if(query.exec(strsql))
    {
        while(query.next())
        {
            online_friends.append(query.value("name").toString());
        }
    }
    return online_friends;
}

QString DbHelper::get_search_user(const QString &name)
{
    QSqlQuery query;
    QString strsql = QString("select name,online from user where name = '%1';").arg(name);
    if(!query.exec(strsql))
    {
        return  MYSQL_ERROR;
    }
    else
    {
        if(query.next())
        {
            if(query.value("online").toInt() == 1)
            {
                return SEARCH_USER_ONLINE;
            }
            else
            {
                return SEARCH_USER_OFFLINE;
            }
        }
        else
        {
            return SEARCH_NO_USER;
        }
    }
}
// 处理用户添加好友请求
/*-1: 发生错误
 * 0: 已经是好友
 * 1: 不是好友，用户在线
 * 2: 不是好友，用户不在线
 * 3: 对方不存在
 */
int DbHelper::handle_add_friend(const QString &src_name, const QString &per_name)
{
    QSqlQuery query;
    QString strsql = QString("select * from friend where (id = (select id from user where name = '%1')"
                             " and friend_id = (select id from user where name = '%2')) "
                             "or (id = (select id from user where name = '%3') "
                             "and friend_id = (select id from user where name = '%4'));").arg(src_name).arg(per_name).arg(per_name).arg(src_name);
    if(query.exec(strsql))
    {
        if(query.next())
        {
            return 0;
        }
        else
        {
            strsql = QString("select name,online from user where name = '%1';").arg(per_name);
            if(query.exec(strsql))
            {
                if(query.next())
                {
                    int online_rst = query.value("online").toInt();
                    if(online_rst == 1)
                    {
                        return 1;
                    }
                    else
                    {
                        return 2;
                    }
                }
                else
                {
                    return 3;
                }
            }
            return -1;
        }
    }
    return -1;
}

bool DbHelper::handle_add_friend_agree(const QString &src_name, const QString &per_name)
{
    QSqlQuery query;
    QString strsql = QString("insert into friend(id,friend_id) values((select id from user where name = '%1')"
                             ",(select id from user where name = '%2'));").arg(src_name).arg(per_name);
    if(query.exec(strsql))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DbHelper::handle_delete_friend(const QString &src_name, const QString &per_name)
{
    QSqlQuery query;
    QString strsql = QString("delete from friend where (id = (select id from user where name = '%1') "
                                            "and friend_id = (select id from user where name = '%2')) "
                                                   "or (id = (select id from user where name = '%3') "
                                            "and friend_id = (select id from user where name = '%4'));").arg(src_name).arg(per_name).arg(per_name).arg(src_name);
    if(query.exec(strsql))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DbHelper::handle_save_fileInfo(const QString& filepath, const QString& md5)
{
    QSqlQuery query;
    QString str_sql = QString("insert into file(filename,md5) values('%1','%2');").arg(filepath).arg(md5);
    if(query.exec(str_sql))
    {
        return true;
    }
    else
    {
        qDebug() << query.lastError().text();
        return  false;
    }
}

QString DbHelper::handle_file_isExited(const QString &md5)
{
    QSqlQuery query;
    QString str_sql = QString("select filename,md5 from file where md5 = '%2';").arg(md5);
    if(query.exec(str_sql))
    {
        if(query.next())
        {
            return  query.value("filename").toString();
        }
        return nullptr;
    }
    else
    {
        qDebug() << query.lastError().text();
        return  nullptr;
    }
}

bool DbHelper::handle_create_share_link(QString filePath, QString link, QString createTime, int timeout)
{
    QSqlQuery query;
    QString strsql = QString("insert into share_link(filePath,link,timein,timeout) values('%1','%2','%3','%4');").arg(filePath).arg(link).arg(createTime).arg(timeout);
    if(query.exec(strsql))
    {
        return true;
    }
    else
    {
        qDebug() << query.lastError().text();
        return  false;
    }
}

QString DbHelper::get_file_by_link(const QString &link, char* filePath)
{
    QSqlQuery query;
    QString strsql = QString("select filePath,timein,timeout from share_link where link = '%1';").arg(link);
    if(query.exec(strsql))
    {
        if(query.next())
        {
            QString dbFilePath = query.value("filePath").toString();
            QString createTime = query.value("timein").toString();
            int timeOut = query.value("timeout").toInt();
            QDateTime currentTime = QDateTime::currentDateTime();
            QDateTime cTime = QDateTime::fromString(createTime,"yyyy-MM-dd hh:mm:ss");
            if((cTime.secsTo(currentTime) > timeOut*24*3600) && (timeOut != 0))
            {
                return GET_FILE_FROM_LINK_TIMEOUT;
            }
            else
            {
                strcpy(filePath,dbFilePath.toStdString().c_str());
                return GET_FILE_FROM_LINK_OK;
            }
        }
        else
        {
            return GET_FILE_FROM_LINK_NOTEXISTED;
        }
    }
    else
    {
        return MYSQL_ERROR;
    }
}

QString DbHelper::generateMd5Hash(const QString &pwd)
{
    QByteArray hash = QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Md5); //
    return QString(hash.toHex());
}
