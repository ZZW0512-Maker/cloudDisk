#include "sharewindow.h"
#include "ui_sharewindow.h"
#include <QListWidget>
#include <QCheckBox>
#include "pack.h"

ShareWindow::ShareWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShareWindow)
{
    ui->setupUi(this);
    init_window();
    /*
    QListWidgetItem *item = new QListWidgetItem;
    QCheckBox *cbb = new QCheckBox;
    cbb->setText("lisi");
    ui->lw_friendlist->addItem(item);
    ui->lw_friendlist->setItemWidget(item,cbb);
*/
}

ShareWindow::~ShareWindow()
{
    delete ui;
}

void ShareWindow::init_window()
{
    this->setWindowTitle(CloudClient::get_instance().get_name());
    this->setWindowIcon(QIcon(":/pg.png"));
    return;
}

void ShareWindow::show_friend_list(QString shareFileName)
{
    m_shareFileName = shareFileName; // 保存要分享的文件名

    /*发送获取在线可分享的好友的请求*/
    Pack pack;
    pack.set_type(TYPE_SHARE_ONLINE_FRIENDS_REQUEST);
    pack.set_content(CloudClient::get_instance().get_name());
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    return;
}

void ShareWindow::handle_online_friends_reponse(const QStringList &content)
{
    //    /*清空列表*/
    //    QListWidgetItem *item = new QListWidgetItem;
    //    QCheckBox *checkbox = new QCheckBox;
    //    while(ui->lw_friendlist->count()>0)
    //    {
    //        item = ui->lw_friendlist->item(ui->lw_friendlist->count()-1);
    //        checkbox = (QCheckBox*)(ui->lw_friendlist->itemWidget(item));
    //        ui->lw_friendlist->removeItemWidget(item);

    //    }
    //    delete item;
    //    delete checkbox;
    //    /*显示最新获取的好友列表*/
    //    for(auto& friend_name : content)
    //    {
    //        if(friend_name == CloudClient::get_instance().get_name())
    //        {
    //            continue;
    //        }
    //        QListWidgetItem *item = new QListWidgetItem;
    //        QCheckBox *checkBox = new QCheckBox;
    //        checkbox->setText(friend_name);
    //        ui->lw_friendlist->addItem(item);
    //        ui->lw_friendlist->setItemWidget(item,checkbox);
    //    }
    // 清空列表
    while (ui->lw_friendlist->count() > 0)
    {
        QListWidgetItem *item = ui->lw_friendlist->takeItem(0); // 使用 takeItem 移除并获取项
        if (item)
        {
            QWidget *widget = ui->lw_friendlist->itemWidget(item); // 获取关联的 widget
            if (widget)
            {
                delete widget; // 删除 widget
            }
            delete item; // 删除 QListWidgetItem
        }
    }

    // 显示最新获取的好友列表
    for (const auto& friend_name : content)
    {
        if (friend_name == CloudClient::get_instance().get_name())
        {
            continue; // 跳过本人用户名
        }

        // 创建新的 QListWidgetItem 和 QCheckBox
        QListWidgetItem *item = new QListWidgetItem;
        QCheckBox *checkbox = new QCheckBox;

        checkbox->setText(friend_name); // 设置好友名称
        ui->lw_friendlist->addItem(item); // 添加到列表
        ui->lw_friendlist->setItemWidget(item, checkbox); // 绑定 widget
    }
    return;
}

void ShareWindow::on_bt_allcheck_clicked()
{
    QListWidgetItem *item = nullptr;
    QCheckBox *checkbox = nullptr;
    for (int i = 0;i<ui->lw_friendlist->count();i++)
    {
        item = ui->lw_friendlist->item(i);
        checkbox = (QCheckBox *)ui->lw_friendlist->itemWidget(item);
        checkbox->setCheckState(Qt::Checked);
    }
    return;
}

void ShareWindow::on_all_notcheck_clicked()
{
    QListWidgetItem *item = nullptr;
    QCheckBox *checkbox = nullptr;
    for (int i = 0;i<ui->lw_friendlist->count();i++)
    {
        item = ui->lw_friendlist->item(i);
        checkbox = (QCheckBox *)ui->lw_friendlist->itemWidget(item);
        checkbox->setCheckState(Qt::Unchecked);
    }
    return;
}

void ShareWindow::on_bt_share_clicked()
{
    /*获取选中的好友*/
    QStringList nameList;
    QListWidgetItem *item = new QListWidgetItem;
    QCheckBox *checkbox = new QCheckBox;
    for(int i=0; i < ui->lw_friendlist->count() ;i++)
    {
        item = ui->lw_friendlist->item(i);
        checkbox = (QCheckBox*)(ui->lw_friendlist->itemWidget(item));
        if(checkbox->checkState() == Qt::Checked)
        {
            nameList.append(checkbox->text());
        }
    }
    QString str_curDir = CloudClient::get_instance().get_current_dir() + "/" + m_shareFileName;
    QString strName = CloudClient::get_instance().get_name();

    Pack pack;
    pack.set_type(TYPE_SHARE_FILE_REQUEST);
    pack.set_content(strName);
    pack.set_content(str_curDir);
    for(auto& name : nameList)
    {
        pack.set_content(name);
    }
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    this->hide();
}

void ShareWindow::on_bt_cancel_clicked()
{
    this->hide();
}
