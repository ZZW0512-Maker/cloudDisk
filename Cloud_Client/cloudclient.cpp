#include "cloudclient.h"
#include "ui_cloudclient.h"
#include "pack.h"
#include <QMessageBox>
#include <QByteArray>
#include <QDebug>
#include <QStringList>
#include "basewindow.h"

CloudClient::CloudClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CloudClient)
    , m_ptcpSocket(new QTcpSocket(this))
    , m_strIP()
    , m_port()
    , m_name()
    , m_currentDir()
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/pg.png"));
    ui->pushButton_login->setDefault(true);
    load_config();
    init_network();
}

CloudClient &CloudClient::get_instance()
{
    static CloudClient instance;
    return instance;
}

CloudClient::~CloudClient()
{
    delete ui;
}

// 加载配置文件，读取IP地址和端口号
void CloudClient::load_config()
{
    // 打开文件
    QFile file(":/client.cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"错误","读取配置文件失败！");
        exit(1);
    }
    QTextStream in(&file); // 创建QTextStream对象以读取文件内容

    m_strIP = in.readLine().trimmed(); // 读取第一行(IP地址)并去除多余的空格或换行符
    m_port = in.readLine().toShort(); // 读取第二行（端口号）

    file.close();  // 关闭文件
}

// 加载配置文件，读取IP地址和端口号
//void CloudClient::load_config()
//{
//    QFile file(":/client.cfg");
//    if(!file.open(QIODevice::ReadOnly))
//    {
//        QMessageBox::warning(this,"错误","读取配置文件失败！");
//        exit(1);
//    }
//    QByteArray data = file.readAll();
//    QString strData = data;
//    qDebug() << strData;
//    strData.replace("\r\n"," ");
//    QStringList strList = strData.split(" ");
//    m_strIP = strList[0];
//    m_port = strList[1].toShort();
//    qDebug() << m_strIP;
//    qDebug() << m_port;
//    file.close();
//}
// 初始化网络连接
void CloudClient::init_network()
{
    m_ptcpSocket->connectToHost(m_strIP,m_port);

    connect(m_ptcpSocket,&QTcpSocket::connected,this,&CloudClient::connect_ok);

    connect(m_ptcpSocket,&QTcpSocket::readyRead,this,&CloudClient::ready_read);
}

QTcpSocket *CloudClient::get_tcpSocket()
{
    return m_ptcpSocket;
}

QString &CloudClient::get_name()
{
    return m_name;
}

QString &CloudClient::get_current_dir()
{
    return m_currentDir;
}

QString CloudClient::set_current_dir(const QString &new_dir)
{
    m_currentDir = new_dir;
    return m_currentDir;
}

// 成功连接服务器
void CloudClient::connect_ok()
{
    // QMessageBox::information(this,"连接","成功连接服务器!");
    ui->pushButton_login->setEnabled(true);
    ui->pushButton_regist->setEnabled(true);
    ui->pushButton_cancal->setEnabled(true);
    return;
}

void CloudClient::ready_read()
{
    while (true)
    {
        Pack pack;
        //qDebug() << pack.get_header_len();
        // 读取完整的包头内容
        QByteArray ba_header = m_ptcpSocket->read(pack.get_header_len());

        // 如果没有完整包头数据，返回等待下一次读取
        if (ba_header.isEmpty())
        {
            return;
        }
        // 将 QByteArray 转换为 QJsonDocument
        QJsonDocument json_header_doc = QJsonDocument::fromJson(ba_header);
        // 检查是否成功解析为 JSON 文档
        if (!json_header_doc.isObject())
        {
            qWarning() << "Invalid JSON format received1";
            return;
        }
        QJsonObject json_header_obj = json_header_doc.object();
        //qint64 header = json_header_obj.value("size").toInt();
        QString str_header = json_header_obj.value("size").toString();
        qint64 header = str_header.toLongLong();
        /***********************************************************************************/
        // 读取完整的 包类型和包内容 数据
        QByteArray json_data = m_ptcpSocket->read(header);
        // 如果没有完整数据，返回等待下一次读取
        if (json_data.isEmpty())
        {
            return;
        }
        // 将 QByteArray 转换为 QJsonDocument
        QJsonDocument json_data_doc = QJsonDocument::fromJson(json_data);

        // 检查是否成功解析为 JSON 文档
        if (!json_data_doc.isObject())
        {
            qWarning() << "Invalid JSON format received";
            return;
        }

        // 获取 JSON 对象
        QJsonObject json_data_obj = json_data_doc.object();

        // 提取 JSON 中的类型字段
        Type type = (Type)json_data_obj.value("type").toInt();

        // 提取 JSON 中的内容字段
        QList<QVariant> variantList = json_data_obj.value("content").toArray().toVariantList(); // 假设这是您的 QList<QVariant>
        QStringList content;

        for (const QVariant &variant : variantList)
        {
            if (variant.canConvert<QString>())
            {
                content.append(variant.toString());
            }
        }
        /***********************************************************************************/
        // 提取二进制数据的长度
        QString str_binary_size = json_data_obj.value("binary_size").toString();
        qint64 binary_size = str_binary_size.toLongLong();
        if (binary_size < 0)
        {
            qWarning() << "Invalid binary_size value";
            return;
        }
        // 读取完整的二进制数据
        QByteArray binary_data = m_ptcpSocket->read(binary_size);

        // 如果没有完整数据，返回等待下一次读取
        if (binary_data.size() < binary_size) {
            qWarning() << "Incomplete binary data received";
            return;
        }

        // 处理二进制数据
       // processBinaryData(binary_data);
        /***********************************************************************************/
        // 根据包类型做相应业务逻辑处理
        switch (type)
        {
        case TYPE_REGIST_REPONSE:
            user_regist_back(content);
            break;
        case TYPE_LOGIN_REPONSE:
            user_login_back(content);
            break;
        case TYPE_CANCEL_REPONSE:
            user_cancel_back(content);
            break;
        case TYPE_ALLONLINE_USERS_REPONSE:
            BaseWindow::get_instance().get_friend_window()->show_allonline_users(content);
            break;
        case TYPE_SEARCH_USER_REPONSE:
            BaseWindow::get_instance().get_friend_window()->show_search_user(content);
            break;
        case TYPE_ADD_FRIEND_REQUEST:
            BaseWindow::get_instance().get_friend_window()->handle_add_friend_request(content);
            break;
        case TYPE_ADD_FRIEND_REPONSE:
            add_friend_back(content);
            break;
        case TYPE_ADD_FRIEND_AGREE:
        case TYPE_ADD_FRIEND_REFUSE:
            BaseWindow::get_instance().get_friend_window()->handle_add_friend_reponse(content);
            break;
        case TYPE_ONLINE_FRIENDS_REPONSE:
            BaseWindow::get_instance().get_friend_window()->show_online_friends(content);
            break;
        case TYPE_DELETE_FRIEND_REPONSE:
            BaseWindow::get_instance().get_friend_window()->handle_delete_friend_reponse(content);
            break;
        case TYPE_SEND_MSG_REPONSE:
            BaseWindow::get_instance().get_friend_window()->handle_rcv_msg(content);
            break;
        case TYPE_CREATE_DIR_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_create_dir_reponse(content);
            break;
        case TYPE_FLUSH_FILE_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_flush_file_reponse(content);
            break;
        case TYPE_DELETE_DIR_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_delete_dir_reponse(content);
            break;
        case TYPE_ENTER_DIR_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_enter_dir_reponse(content);
            break;
        case TYPE_RENAME_DIR_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_rename_dir_reponse(content);
            break;
        case TYPE_UPLOAD_FILE_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_upload_file_reponse(content);
            break;
        case TYPE_UPLOAD_FILE_CONTENT_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_upload_file_content_reponse(content);
            break;
        case TYPE_DOWNLOAD_FILE_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_download_file_reponse(content);
            break;
        case TYPE_DOWNLOAD_FILE_CONTENT_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_download_file_content_reponse(content,binary_data);
            break;
        case TYPE_MOVE_FILE_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_move_file_reponse(content);
            break;
        case TYPE_SHARE_ONLINE_FRIENDS_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_online_friend_reponse(content);
            break;
        case TYPE_SHARE_FILE_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_share_file_reponse(content);
            break;
        case TYPE_SHARE_FILE_REQUEST:
            BaseWindow::get_instance().get_file_window()->handle_share_file_request(content);
            break;
        case TYPE_SHARE_FILE_LINK_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_share_file_link_reponse(content);
            break;
        case TYPE_GET_FILE_FROM_LINK_REPONSE:
            BaseWindow::get_instance().get_file_window()->handle_get_file_from_link_reponse(content);
            break;
        default:
            break;
        }
    }
}

// 处理用户点击登录按钮
void CloudClient::on_pushButton_login_clicked()
{
    if(ui->lineEdit_name->text().isEmpty() || ui->lineEdit_pwd->text().isEmpty())
    {
        QMessageBox::information(this,"提示","用户名或密码不能为空！");
        return;
    }
    QString name = ui->lineEdit_name->text();
    QString pwd = ui->lineEdit_pwd->text();

    m_name = name;

    Pack pack;
    pack.set_type(TYPE_LOGIN_REQUEST);
    pack.set_content(name);
    pack.set_content(pwd);
    m_ptcpSocket->write(pack.data());

    return;
}

// 处理用户点击注册按钮
void CloudClient::on_pushButton_regist_clicked()
{
    if(ui->lineEdit_name->text().isEmpty() || ui->lineEdit_pwd->text().isEmpty())
    {
        QMessageBox::information(this,"提示","用户名或密码不能为空！");
        return;
    }
    QString name = ui->lineEdit_name->text();
    QString pwd = ui->lineEdit_pwd->text();
    ui->lineEdit_name->clear();
    ui->lineEdit_pwd->clear();

    Pack pack;
    pack.set_type(TYPE_REGIST_REQUEST);
    pack.set_content(name);
    pack.set_content(pwd);
    m_ptcpSocket->write(pack.data());

    return;
}

void CloudClient::on_pushButton_cancal_clicked()
{
    if(ui->lineEdit_name->text().isEmpty() || ui->lineEdit_pwd->text().isEmpty())
    {
        QMessageBox::information(this,"提示","用户名或密码不能为空！");
        return;
    }
    QString name = ui->lineEdit_name->text();
    QString pwd = ui->lineEdit_pwd->text();
    ui->lineEdit_name->clear();
    ui->lineEdit_pwd->clear();

    Pack pack;
    pack.set_type(TYPE_CANCEL_REQUEST);
    pack.set_content(name);
    pack.set_content(pwd);
    m_ptcpSocket->write(pack.data());
}

void CloudClient::user_regist_back(const QStringList &content)
{
    QString register_rst = content[0]; // 获取注册结果
    if(register_rst == USER_REGIST_OK)
    {
        QMessageBox::information(this,"注册","注册成功！");
    }
    else if(register_rst == USER_REGIST_FAILD_1)
    {
        QMessageBox::warning(this,"注册","注册失败，重复注册！");
    }
    else if(register_rst == MYSQL_ERROR)
    {
        QMessageBox::warning(this,"注册","注册失败，数据库异常错误！");
    }
    else
    {
        QMessageBox::warning(this,"注册","注册失败，未知错误!");
    }
    return;
}

void CloudClient::user_login_back(const QStringList &content)
{
    QString register_rst = content[0]; // 获取登录结果

    if(register_rst == USER_LOGIN_OK)
    {
        //QMessageBox::information(this,"登录","登录成功！");
        m_currentDir = m_name;
        BaseWindow::get_instance().show();
        BaseWindow::get_instance().setWindowTitle(m_name);
        BaseWindow::get_instance().setWindowIcon(QIcon(":/pg.png"));
        this->hide();
    }
    else if(register_rst == USER_LOGIN_FAILD_1)
    {
        QMessageBox::warning(this,"登录","登录失败，用户名或密码错误！");
    }
    else if(register_rst == MYSQL_ERROR)
    {
        QMessageBox::warning(this,"登录","登录失败，数据库异常错误！");
    }
    else if(register_rst == USER_LOGIN_FAILD_3)
    {
        QMessageBox::warning(this,"登录","登录失败，重复登录！");
    }
    else
    {
        QMessageBox::warning(this,"登录","登录失败，未知错误!");
    }
    return;
}

void CloudClient::user_cancel_back(const QStringList &content)
{
    QString cancel_rst = content[0]; // 获取登录结果

    if(cancel_rst == USER_CANCEL_OK)
    {
        QMessageBox::information(this,"注销","注销成功！");
    }
    else if(cancel_rst == USER_CANCEL_FAILD_1)
    {
        QMessageBox::warning(this,"注销","注销失败，用户名或密码错误！");
    }
    else if(cancel_rst == MYSQL_ERROR)
    {
        QMessageBox::warning(this,"注销","注销失败，数据库异常错误！");
    }
    else
    {
        QMessageBox::warning(this,"注销","注销失败，未知错误!");
    }
    return;
}

void CloudClient::add_friend_back(const QStringList &content)
{
    QString add_rst = content[0];

    if(add_rst == MYSQL_ERROR)
    {
        QMessageBox::information(this,"添加好友","数据库错误");
    }
    else if(add_rst == ADD_FRIEND_EXISTED)
    {
        QMessageBox::information(this,"添加好友","已经是好友，不能重复添加!");
    }
    else if(add_rst == ADD_FRIEND_OFFLINE)
    {
        QMessageBox::information(this,"添加好友","不是好友，但对方不在线!");
    }
    else if(add_rst == ADD_FRIEND_USER_NOEXISTED)
    {
        QMessageBox::information(this,"添加好友","用户不存在!");
    }
}
