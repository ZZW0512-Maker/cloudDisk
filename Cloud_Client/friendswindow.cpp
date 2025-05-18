#include "friendswindow.h"
#include "ui_friendswindow.h"
#include "pack.h"
#include "cloudclient.h"
#include <QInputDialog>
#include <QMessageBox>

FriendsWindow::FriendsWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendsWindow)
  , m_searchname()
  , flag()
{
    ui->setupUi(this);

}

FriendsWindow::~FriendsWindow()
{
    delete ui;
}

void FriendsWindow::handle_add_friend_request(const QStringList &content)
{
    QString src_name = content[0];
    Pack pack;
    int ret = QMessageBox::information(this,"添加好友",QString("%1 添加你为好友，是否同意？").arg(src_name),QMessageBox::Yes,QMessageBox::No);
    if(QMessageBox::Yes == ret)
    {
        pack.set_type(TYPE_ADD_FRIEND_AGREE);
    }
    else
    {
        pack.set_type(TYPE_ADD_FRIEND_REFUSE);
    }
    pack.set_content(src_name);
    pack.set_content(CloudClient::get_instance().get_name());
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
}

void FriendsWindow::handle_add_friend_reponse(const QStringList &content)
{
    if(content[0] == "agree")
    {
        QMessageBox::information(this,"添加好友",QString("成功添加%1为好友").arg(content[1]));
    }
    else
    {
        QMessageBox::information(this,"添加好友",QString("%1 拒绝添加你为好友").arg(content[1]));
    }
    return;
}

void FriendsWindow::handle_delete_friend_reponse(const QStringList &content)
{
    QString del_rst = content[0];
    if(del_rst == DELETE_FRIEND_OK)
    {
        QMessageBox::information(this,"删除好友","删除成功");
    }
    else
    {
        QMessageBox::information(this,"删除好友","删除失败");
    }
    return;
}

void FriendsWindow::handle_rcv_msg(const QStringList &content)
{
    QString snd_name = content[0];
    QString rcv_msg = content[1].toStdString().c_str();
    ui->textEdit_rcvmsg->append(snd_name + ":" + rcv_msg);
    return;
}

void FriendsWindow::show_allonline_users(const QStringList &content)
{
    ui->listWidget_user->clear();
    for(auto& name : content)
    {
        if(name == CloudClient::get_instance().get_name())
        {
            continue;
        }
        ui->listWidget_user->addItem(name);
    }
    return;
}

void FriendsWindow::show_online_friends(const QStringList &content)
{
    ui->listWidget_user->clear();
    for(auto& name : content)
    {
        ui->listWidget_user->addItem(name);
    }
    return;
}

void FriendsWindow::show_search_user(const QStringList &content)
{
    ui->listWidget_user->clear();
    QString search_rst = content[0];
    if(search_rst == SEARCH_USER_ONLINE)
    {
        ui->listWidget_user->addItem(m_searchname);
    }
    else if(search_rst == SEARCH_NO_USER)
    {
        QMessageBox::information(this,"搜索结果","该用户不存在！");
    }
    else if(search_rst == SEARCH_USER_OFFLINE)
    {
        QMessageBox::information(this,"搜索结果","用户不在线！");
    }
    else if(search_rst == MYSQL_ERROR)
    {
        QMessageBox::warning(this,"错误","数据库错误！");
    }
    return;
}

void FriendsWindow::on_bt_allonline_clicked()
{
    flag = 0;
    Pack pack;
    pack.set_type(TYPE_ALLONLINE_USERS_REQUEST);
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    return;
}

void FriendsWindow::on_bt_onlinefriend_clicked()
{
    flag = 1;
    Pack pack;
    pack.set_type(TYPE_ONLINE_FRIENDS_REQUEST);
    pack.set_content(CloudClient::get_instance().get_name());
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
}

void FriendsWindow::on_bt_search_clicked()
{
    flag = 0;
    QString name = QInputDialog::getText(this,"搜索用户","用户名：");
    if(!name.isEmpty())
    {
        m_searchname = name;
        Pack pack;
        pack.set_type(TYPE_SEARCH_USER_REQUEST);
        pack.set_content(name);
        CloudClient::get_instance().get_tcpSocket()->write(pack.data());
        return;
    }
}

void FriendsWindow::on_bt_add_friend_clicked()
{
    flag = 0;
    QListWidgetItem *item = ui->listWidget_user->currentItem();
    if(item != NULL)
    {
        QString add_friend_name = item->text();
        Pack pack;
        pack.set_type(TYPE_ADD_FRIEND_REQUEST);
        pack.set_content(add_friend_name);
        CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    }
    else
    {
        QMessageBox::information(this,"提示","请先选中需要添加为好友的用户！");
    }
    return;
}

void FriendsWindow::on_bt_del_friend_clicked()
{
    flag = 0;
    QListWidgetItem *item = ui->listWidget_user->currentItem();
    if(item != NULL)
    {
        QString delete_friend_name = item->text();
        Pack pack;
        pack.set_type(TYPE_DELETE_FRIEND_REQUEST);
        pack.set_content(delete_friend_name);
        CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    }
    else
    {
        QMessageBox::information(this,"提示","请先选中需要删除的好友!");
    }
    return;
}

void FriendsWindow::on_bt_send_msg_clicked()
{
    if(flag == 1)
    {
        QList<QListWidgetItem*> select_list = ui->listWidget_user->selectedItems();
        int friend_count = select_list.size();
        if(friend_count > 0)
        {
            QString strmsg = ui->textEdit_sndmsg->toPlainText();
            if(strmsg.isEmpty())
            {
                QMessageBox::information(this,"发送消息","发送的消息为空");
                return;
            }
            Pack pack;
            pack.set_type(TYPE_SEND_MSG_REQUEST);
            pack.set_content(strmsg);
            pack.set_content(CloudClient::get_instance().get_name());
            for(auto& friend_name : select_list)
            {
                pack.set_content(friend_name->text());
            }
            CloudClient::get_instance().get_tcpSocket()->write(pack.data());
        }
        else
        {
            QMessageBox::information(this,"发送消息","请至少选择一个好友");
        }
    }
    else
    {
        QMessageBox::information(this,"发送消息","请在好友列表中选择好友");
    }
    return;
}
