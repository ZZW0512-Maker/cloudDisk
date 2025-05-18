#ifndef PACK_H
#define PACK_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QJsonArray>

typedef unsigned int uint;

// 包类型
enum Type
{
    TYPE_MIN = 0,
    TYPE_REGIST_REQUEST,/*注册账号请求*/
    TYPE_REGIST_REPONSE,/*注册账号回复*/
    TYPE_LOGIN_REQUEST,/*登录账号请求*/
    TYPE_LOGIN_REPONSE,/*登录账号回复*/
    TYPE_CANCEL_REQUEST,/*注销账号请求*/
    TYPE_CANCEL_REPONSE,/*注销账号回复*/

    TYPE_ALLONLINE_USERS_REQUEST,/*查询所有在线用户请求*/
    TYPE_ALLONLINE_USERS_REPONSE,/*查询所有在线用户回复*/
    TYPE_ONLINE_FRIENDS_REQUEST,/*查询在线好友请求*/
    TYPE_ONLINE_FRIENDS_REPONSE,/*查询在线好友回复*/
    TYPE_SEARCH_USER_REQUEST,/*搜索用户请求*/
    TYPE_SEARCH_USER_REPONSE,/*搜索用户回复*/

    TYPE_ADD_FRIEND_REQUEST,/*添加好友请求*/
    TYPE_ADD_FRIEND_REPONSE,/*添加好友回复*/
    TYPE_ADD_FRIEND_AGREE,/*同意添加好友*/
    TYPE_ADD_FRIEND_REFUSE,/*拒绝添加好友*/
    TYPE_DELETE_FRIEND_REQUEST,/*删除好友请求*/
    TYPE_DELETE_FRIEND_REPONSE,/*删除好友回复*/
    TYPE_SEND_MSG_REQUEST,/*发送消息请求*/
    TYPE_SEND_MSG_REPONSE,/*发送消息回复*/

    TYPE_CREATE_DIR_REQUEST,/*创建文件夹请求*/
    TYPE_CREATE_DIR_REPONSE,/*创建文件夹回复*/

    TYPE_FLUSH_FILE_REQUEST,/*刷新文件请求*/
    TYPE_FLUSH_FILE_REPONSE,/*刷新文件回复*/
    TYPE_DELETE_DIR_REQUEST,/*删除文件夹请求*/
    TYPE_DELETE_DIR_REPONSE,/*删除文件夹回复*/

    TYPE_ENTER_DIR_REQUEST,/*进入文件夹请求*/
    TYPE_ENTER_DIR_REPONSE,/*进入文件夹回复*/

    TYPE_RENAME_DIR_REQUEST,/*重命名文件夹请求*/
    TYPE_RENAME_DIR_REPONSE,/*重命名文件夹回复*/

    TYPE_UPLOAD_FILE_REQUEST,/*上传文件请求*/
    TYPE_UPLOAD_FILE_REPONSE,/*上传文件回复*/
    TYPE_UPLOAD_FILE_CONTENT_REQUEST,/*上传文件内容请求*/
    TYPE_UPLOAD_FILE_CONTENT_REPONSE,/*上传文件内容回复*/

    TYPE_DOWNLOAD_FILE_REQUEST,/*下载文件请求*/
    TYPE_DOWNLOAD_FILE_REPONSE,/*下载文件回复*/
    TYPE_DOWNLOAD_FILE_CONTENT_REQUEST,/*下载文件内容请求*/
    TYPE_DOWNLOAD_FILE_CONTENT_REPONSE,/*下载文件内容回复*/

    TYPE_MOVE_FILE_REQUEST,/*移动文件请求*/
    TYPE_MOVE_FILE_REPONSE,/*移动文件回复*/

    TYPE_SHARE_ONLINE_FRIENDS_REQUEST,/*查询在线好友请求*/
    TYPE_SHARE_ONLINE_FRIENDS_REPONSE,/*查询在线好友回复*/

    TYPE_SHARE_FILE_REQUEST,/*分享文件请求*/
    TYPE_SHARE_FILE_REPONSE,/*分享文件回复*/

    TYPE_SHARE_FILE_AGREE,/*同意分享文件*/
    TYPE_SHARE_FILE_REFUSE,/*拒绝分享文件*/

    TYPE_SHARE_FILE_LINK_REQUEST,/*分享文件链接请求*/
    TYPE_SHARE_FILE_LINK_REPONSE,/*分享文件链接回复*/

    TYPE_GET_FILE_FROM_LINK_REQUEST,/*从链接获取文件请求*/
    TYPE_GET_FILE_FROM_LINK_REPONSE,/*从链接获取文件回复*/

    TYPE_MAX = 0X00FFFFFF
};

#define MYSQL_ERROR "MYSQL_ERROR"                                       // 数据库错误
#define USER_REGIST_OK "USER_REGIST_OK"                                 // 注册成功
#define USER_REGIST_FAILD_1 "USER_REGIST_FAILD:NAME EXISTED"            // 重复注册
#define USER_LOGIN_OK "USER_LOGIN_OK"                                   // 登录成功
#define USER_LOGIN_FAILD_1 "USER_LOGIN_FAILD:NAME OR PASSWD ERROR"      // 用户名或密码错误
#define USER_LOGIN_FAILD_3 "USER_LOGIN_FAILD:USER ONLINE"               // 用户在线重复登录
#define USER_CANCEL_OK "USER_CANCEL_OK"                                 // 注销成功
#define USER_CANCEL_FAILD_1 "USER_CANCEL_FAILD:NAME OR NAME ERROR"      // 用户名或密码错误
#define SEARCH_NO_USER "SEARCH_NO_USER"                                 // 查无此用户
#define SEARCH_USER_ONLINE "SEARCH_USER_ONLINE"                         // 查到用户在线
#define SEARCH_USER_OFFLINE "SEARCH_USER_OFFLINE"                       // 查到用户不在线
#define ADD_FRIEND_EXISTED "FRIEND EXISTED"                             // 已经是好友
#define ADD_FRIEND_OFFLINE "USER OFFLINE"                               // 添加的用户不在线
#define ADD_FRIEND_USER_NOEXISTED "USER NOEXISTED"                      // 添加的用户不存在
#define DELETE_FRIEND_OK "DELETE_FRIEND_OK"                              // 成功删除好友

#define CREATE_DIR_NOEXISTED "CREATE_DIR_NOEXISTED"                     // 文件夹路径不存在
#define CREATE_DIR_OK "CREATE_DIR_OK"                                   // 成功创建文件夹
#define CREATE_DIR_EXISTED "CREATE_DIR_EXISTED"                         // 文件夹已存在

#define DELETE_DIR_OK "DELETE_DIR_OK"                                   // 成功删除文件夹
#define DELETE_NOT_DIR "NOT DIR" "NOT DIR"                              // 不是一个路径

#define ENTER_DIR_IS_DIR "ISDIR"                                        // 是一个目录
#define ENTER_DIR_NOT_DIR "NOTDIR"                                      // 不是一个目录
#define RENAME_OK "RENAME_DIR_OK"                                       // 重命名成功
#define RENAME_ERROR "RENAME_DIR_ERROR"                                 // 重命名失败

#define UPLOAD_FILE_OPEN_OK "UPLOAD_FILE_OPEN_OK"                       // 成功打开上传文件
#define UPLOAD_FILE_OPEN_ERROR "UPLOAD_FILE_OPEN_ERROR"                 // 打开上传文件失败
#define UPLOAD_FILE_COMPLETE "UPLOAD_FILE_COMPLETE"                     // 文件上传完成
#define UPLOAD_FILE_FAILD "UPLOAD_FILE_FAILD"                           // 文件上传失败
#define UPLOAD_FILE_CHECK_FAILD "UPLOAD_FILE_CHECK_FAILD"               // 上传文件校验失败
#define UPLOAD_FILE_EXITED_OK "UPLOAD_FILE_EXITED_OK"                   // 上传文件已存在
#define DOWN_LOAD_FILE_ISDIR "DOWN_LOAD_FILE_ISDIR"                     // 下载的文件是文件夹
#define MOVE_FILE_OK "MOVE_FILE_OK"
#define MOVE_FILE_FAILED "MOVE_FILE_FAILED"

#define GET_FILE_FROM_LINK_OK "GET_FILE_FROM_LINK_OK"
#define GET_FILE_FROM_LINK_NOTEXISTED "GET_FILE_FROM_LINK_NOTEXISTED"
#define GET_FILE_FROM_LINK_TIMEOUT "GET_FILE_FROM_LINK_TIMEOUT"
#define GET_FILE_FROM_LINK_FILENOTEXISTED "GET_FILE_FROM_LINK_FILENOTEXISTED"

class Pack
{
public:
    Pack();

public:
    QByteArray data(); // 对包类型进行序列化

public:
    void clear(); // 清空包

    uint get_header_len(); // 获取包头值序列化之后的长度

    void set_content(const QString& data); // 设置包内容

    void set_binary_size(quint64 len);   // 设置二进制数据长度

    void set_type(const Type& type); // 设置包类型

private:
    uint m_size; //序列化之后包类型和包内容的长度
    Type m_type; // 包类型
    QStringList m_content; // 包内容

    uint m_binary_size; // 二进制数据的大小
};

#endif // PACK_H
