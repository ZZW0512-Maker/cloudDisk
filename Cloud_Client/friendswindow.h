#ifndef FRIENDSWINDOW_H
#define FRIENDSWINDOW_H

#include <QWidget>
#include <QStringList>

namespace Ui {
class FriendsWindow;
}

class FriendsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FriendsWindow(QWidget *parent = nullptr);
    ~FriendsWindow();

public:
    void handle_add_friend_request(const QStringList& content);      // 处理添加好友请求
    void handle_add_friend_reponse(const QStringList& content);      // 处理添加好友回复
    void handle_delete_friend_reponse(const QStringList& content);   // 处理删除好友回复
    void handle_rcv_msg(const QStringList& content);                 // 处理接收到消息

public:
    void show_allonline_users(const QStringList& content);   // 在用户列表展示在线用户
    void show_online_friends(const QStringList& content);   // 在用户列表展示在线好友
    void show_search_user(const QStringList& content);      // 在用户列表展示搜索的用户

private slots:
    void on_bt_allonline_clicked();

    void on_bt_onlinefriend_clicked();

    void on_bt_search_clicked();

    void on_bt_add_friend_clicked();

    void on_bt_del_friend_clicked();

    void on_bt_send_msg_clicked();

private:
    Ui::FriendsWindow *ui;
    QString m_searchname;
    int flag;
};

#endif // FRIENDSWINDOW_H
