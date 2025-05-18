#include "cloudserve.h"
#include "ui_cloudserve.h"
#include "dbhelper.h"
#include "pack.h"
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QFileInfoList>
#include <QCryptographicHash>
#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <qt_windows.h>
#include <QUuid>
#include <QDateTime>

CloudServe::CloudServe(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CloudServe)
    , m_ptcpServe(new QTcpServer(this))
    , m_pmyTcpSockt_vec()
    , m_strIP()
    , m_port()
{
    ui->setupUi(this);
    init_db();
    load_config();
    init_nerwork();
}

CloudServe &CloudServe::get_instance()
{
    static CloudServe instance;
    return instance;
}

CloudServe::~CloudServe()
{
    delete ui;
}
// 连接数据库
void CloudServe::init_db()
{
    if(!DbHelper::get_instance().init_db())
    {
        QMessageBox::warning(this,"错误","数据库连接失败！");
        exit(1);
    }
    return;
}

// 加载配置文件
void CloudServe::load_config()
{
    // 打开文件
    QFile file(":/serve.cfg");
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

void CloudServe::init_nerwork()
{

    if(!m_ptcpServe->listen(QHostAddress::Any,m_port))
    {
        QMessageBox::warning(this,"错误","监听网络端口失败！");
        exit(2);
    }
    connect(m_ptcpServe,&QTcpServer::newConnection,this,&CloudServe::new_connection);
    return ;
}

// 新客户端连接处理
void CloudServe::new_connection()
{
    MyTcpSocket *pmyTcpSocket = new MyTcpSocket(m_ptcpServe->nextPendingConnection());

    m_pmyTcpSockt_vec.push_back(pmyTcpSocket);

    // 接收到客户端消息
    connect(pmyTcpSocket->get_tcpsocket(),&QTcpSocket::readyRead,this,&CloudServe::ready_read);
    // 客户端下线
    connect(pmyTcpSocket->get_tcpsocket(),&QTcpSocket::disconnected,this,&CloudServe::dis_connected);
}

// 客户端下线处理
void CloudServe::dis_connected()
{
    QTcpSocket *ptcpSocket = static_cast<QTcpSocket *>(sender());

    MyTcpSocket *pmyTcpSocket = find_pmyTcpSocket(ptcpSocket);

    if(DbHelper::get_instance().user_offline(pmyTcpSocket->get_name()))
    {
        m_pmyTcpSockt_vec.removeOne(pmyTcpSocket);

        pmyTcpSocket->close();

        delete pmyTcpSocket;
    }
    return;
}
// 接收客户端消息
void CloudServe::ready_read()
{
    QTcpSocket *ptcpSocket = static_cast<QTcpSocket *>(sender());

    MyTcpSocket *pmyTcpSocket = find_pmyTcpSocket(ptcpSocket);
#if 0
    if(pmyTcpSocket->get_uploading())
    {
        Pack pack;
        pack.set_type(TYPE_UPLOAD_FILE_REPONSE);
        /*将socket的内容全部读取出来*/
        QByteArray buffer = pmyTcpSocket->readAll();
        pmyTcpSocket->get_uploadFile()->write(buffer); // 写入文件
        pmyTcpSocket->setUploadFileRecived(pmyTcpSocket->getUploadFileRecived()+buffer.size()); // 记录接收文件内容的大小

        /*当接收的大小 = 文件总大小则认为接收完成*/
        if(pmyTcpSocket->getUploadFileSize() <= pmyTcpSocket->getUploadFileRecived())
        {
            /*文件上传完成*/
            pmyTcpSocket->set_uploading(false);
            pmyTcpSocket->get_uploadFile()->close();

            /*MD5校验*/
            pmyTcpSocket->get_uploadFile()->open(QFile::ReadOnly);
            QByteArray array = pmyTcpSocket->get_uploadFile()->readAll();
            pmyTcpSocket->get_uploadFile()->close();
            QByteArray md5_array =  QCryptographicHash::hash(array,QCryptographicHash::Md5);
            QString md5_str = md5_array.toHex().constData();

            if(md5_str == pmyTcpSocket->getUploadFileMd5())
            {
                /*把文件信息存储到数据库表中*/
                QString fileName = pmyTcpSocket->get_uploadFile()->fileName();
                QString db_md5 = pmyTcpSocket->getUploadFileMd5().toStdString().c_str();
                bool res = DbHelper::get_instance().handle_save_fileInfo(fileName.toStdString().c_str(),db_md5);
                int index = fileName.lastIndexOf("/");
                fileName.remove(0,index+1);
                /*发送上传完成消息*/
                if(res)
                {
                    QString fileLnkName = fileName + ".lnk";
                    QString linkFilePath = pmyTcpSocket->getCurrentPath() + "/" + fileLnkName;
                    //createShortcut(pmyTcpSocket->get_uploadFile()->fileName(), linkFilePath);
                    pmyTcpSocket->get_uploadFile()->link(linkFilePath);
                    pack.set_content(UPLOAD_FILE_COMPLETE);
                }
                else
                {
                    pack.set_content(UPLOAD_FILE_FAILD);
                }
            }
            else
            {
                pack.set_content(UPLOAD_FILE_CHECK_FAILD);
                pmyTcpSocket->get_uploadFile()->remove();
            }
            pmyTcpSocket->write(pack.data());
        }
        return;
    }
#endif
    while (true)
    {
        Pack pack;
        //qDebug() << pack.get_header_len();
        // 读取完整的包头内容
        QByteArray ba_header = pmyTcpSocket->read(pack.get_header_len());

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
        QByteArray json_data = pmyTcpSocket->read(header);
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
            qWarning() << "Invalid JSON format received2";
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
        //        if (binary_size <= 0)
        //        {
        //            qWarning() << "Invalid binary_size value";
        //            return;
        //        }
        // 读取完整的二进制数据
        QByteArray binary_data = pmyTcpSocket->read(binary_size);

        // 如果没有完整数据，返回等待下一次读取
        if (binary_data.size() < binary_size)
        {
            qWarning() << "Incomplete binary data received";
            return;
        }

        // 处理二进制数据
        // processBinaryData(binary_data);
        /***********************************************************************************/

        // 根据包类型做相应业务逻辑处理
        switch (type)
        {
        case TYPE_REGIST_REQUEST:
            handle_user_regist(pmyTcpSocket,content);
            break;
        case TYPE_LOGIN_REQUEST:
            handle_user_login(pmyTcpSocket,content);
            break;
        case TYPE_CANCEL_REQUEST:
            handle_user_cancel(pmyTcpSocket,content);
            break;
        case TYPE_ALLONLINE_USERS_REQUEST:
            handle_allonline_users(pmyTcpSocket,content);
            break;
        case TYPE_SEARCH_USER_REQUEST:
            handle_search_user(pmyTcpSocket,content);
            break;
        case TYPE_ADD_FRIEND_REQUEST:
            handle_add_friend(pmyTcpSocket,content);
            break;
        case TYPE_ADD_FRIEND_AGREE:
            handle_add_friend_agree(pmyTcpSocket,content);
            break;
        case TYPE_ADD_FRIEND_REFUSE:
            handle_add_friend_refuse(pmyTcpSocket,content);
            break;
        case TYPE_ONLINE_FRIENDS_REQUEST:
            handle_online_friends(pmyTcpSocket,content);
            break;
        case TYPE_DELETE_FRIEND_REQUEST:
            handle_delete_friend(pmyTcpSocket,content);
            break;
        case TYPE_SEND_MSG_REQUEST:
            handle_send_msg(pmyTcpSocket,content);
            break;
        case TYPE_CREATE_DIR_REQUEST:
            handle_create_dir(pmyTcpSocket,content);
            break;
        case TYPE_FLUSH_FILE_REQUEST:
            handle_flush_file(pmyTcpSocket,content);
            break;
        case TYPE_DELETE_DIR_REQUEST:
            handle_delete(pmyTcpSocket,content);
            break;
        case TYPE_ENTER_DIR_REQUEST:
            handle_enter_dir(pmyTcpSocket,content);
            break;
        case TYPE_RENAME_DIR_REQUEST:
            handle_rename_dir(pmyTcpSocket,content);
            break;
        case TYPE_UPLOAD_FILE_REQUEST:
            handle_upload_file(pmyTcpSocket,content);
            break;
        case TYPE_UPLOAD_FILE_CONTENT_REQUEST:
            handle_upload_file_content(pmyTcpSocket,content,binary_data);
            break;
        case TYPE_DOWNLOAD_FILE_REQUEST:
            handle_download_file(pmyTcpSocket,content);
            break;
        case TYPE_DOWNLOAD_FILE_CONTENT_REQUEST:
            handle_download_file_content(pmyTcpSocket,content);
            break;
        case TYPE_MOVE_FILE_REQUEST:
            handle_move_file(pmyTcpSocket,content);
            break;
        case TYPE_SHARE_ONLINE_FRIENDS_REQUEST:
            handle_share_online_friends(pmyTcpSocket,content);
            break;
        case TYPE_SHARE_FILE_REQUEST:
            handle_share_file(pmyTcpSocket,content);
            break;
        case TYPE_SHARE_FILE_AGREE:
            handle_share_file_agree(pmyTcpSocket,content);
            break;
        case TYPE_SHARE_FILE_LINK_REQUEST:
            handle_share_file_link(pmyTcpSocket,content);
            break;
        case TYPE_GET_FILE_FROM_LINK_REQUEST:
            handle_get_file_from_link(pmyTcpSocket,content);
            break;
        default:
            break;
        }
    }
}
// 处理用户注册
void CloudServe::handle_user_regist(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString name = content[0];
    QString pwd = content[1];

    QString regist_rst = DbHelper::get_instance().user_regist(name,pwd);

    if(regist_rst == USER_REGIST_OK)
    {
        QDir dir;
        dir.mkdir(QString("./%1").arg(name));
    }

    Pack pack;
    pack.set_type(TYPE_REGIST_REPONSE);
    pack.set_content(regist_rst);
    pmyTcpSocket->write(pack.data());
    return;
}
// 处理用户登录
void CloudServe::handle_user_login(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString name = content[0];
    QString pwd = content[1];

    QString login_rst = DbHelper::get_instance().user_login(name,pwd);
    if(login_rst == USER_LOGIN_OK)
    {
        pmyTcpSocket->set_name(name);
    }
    Pack pack;
    pack.set_type(TYPE_LOGIN_REPONSE);
    pack.set_content(login_rst);
    pmyTcpSocket->write(pack.data());
    return;
}
// 处理用户注销
void CloudServe::handle_user_cancel(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString name = content[0];
    QString pwd = content[1];

    QString cancel_rst = DbHelper::get_instance().user_cancel(name,pwd);

    Pack pack;
    pack.set_type(TYPE_CANCEL_REPONSE);
    pack.set_content(cancel_rst);
    pmyTcpSocket->write(pack.data());
    return;
}
// 处理用户全部在线用户请求
void CloudServe::handle_allonline_users(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QStringList users = DbHelper::get_instance().get_allonline_users();
    Pack pack;
    pack.set_type(TYPE_ALLONLINE_USERS_REPONSE);
    for(auto& name : users)
    {
        pack.set_content(name);
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_search_user(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString search_name = content[0];
    QString search_rst = DbHelper::get_instance().get_search_user(search_name);
    Pack pack;
    pack.set_type(TYPE_SEARCH_USER_REPONSE);
    pack.set_content(search_rst);
    pmyTcpSocket->write(pack.data());
    return;
}
// 处理添加好友
/*-1: 发生错误
 * 0: 已经是好友
 * 1: 不是好友，用户在线
 * 2: 不是好友，用户不在线
 * 3: 对方不存在
 */
void CloudServe::handle_add_friend(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString src_name = pmyTcpSocket->get_name();
    QString per_name = content[0];
    int add_rst = DbHelper::get_instance().handle_add_friend(src_name,per_name);

    Pack pack;
    pack.set_type(TYPE_ADD_FRIEND_REPONSE);
    if(add_rst == -1)
    {
        pack.set_content(MYSQL_ERROR);
        pmyTcpSocket->write(pack.data());
    }
    else if(add_rst == 0)
    {
        pack.set_content(ADD_FRIEND_EXISTED);
        pmyTcpSocket->write(pack.data());
    }
    else if(add_rst == 1)
    {
        for(auto potherTcpSocket : m_pmyTcpSockt_vec)
        {
            if(potherTcpSocket->get_name() == per_name)
            {
                Pack pack;
                pack.set_type(TYPE_ADD_FRIEND_REQUEST);
                pack.set_content(src_name);
                potherTcpSocket->write(pack.data());
            }
        }
    }
    else if(add_rst == 2)
    {
        pack.set_content(ADD_FRIEND_OFFLINE);
        pmyTcpSocket->write(pack.data());
    }
    else if(add_rst == 3)
    {
        pack.set_content(ADD_FRIEND_USER_NOEXISTED);
        pmyTcpSocket->write(pack.data());
    }
}

void CloudServe::handle_add_friend_agree(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString src_name = content[0];
    QString per_name = content[1];
    bool agree_rst = DbHelper::get_instance().handle_add_friend_agree(src_name,per_name);
    if(agree_rst)
    {
        for(auto potherTcpSocket : m_pmyTcpSockt_vec)
        {
            if(potherTcpSocket->get_name() == src_name)
            {
                Pack pack;
                pack.set_type(TYPE_ADD_FRIEND_AGREE);
                pack.set_content("agree");
                pack.set_content(per_name);
                potherTcpSocket->write(pack.data());
            }
        }
    }
    return;
}

void CloudServe::handle_add_friend_refuse(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString src_name = content[0];
    QString per_name = content[1];
    for(auto potherTcpSocket : m_pmyTcpSockt_vec)
    {
        if(potherTcpSocket->get_name() == src_name)
        {
            Pack pack;
            pack.set_type(TYPE_ADD_FRIEND_REFUSE);
            pack.set_content("refuse");
            pack.set_content(per_name);
            potherTcpSocket->write(pack.data());
        }
    }
}

void CloudServe::handle_online_friends(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString user_name = content[0];
    QStringList online_friends = DbHelper::get_instance().get_online_friends(user_name);
    Pack pack;
    pack.set_type(TYPE_ONLINE_FRIENDS_REPONSE);
    for(auto& name : online_friends)
    {
        pack.set_content(name);
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_delete_friend(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString src_name = pmyTcpSocket->get_name();
    QString per_name = content[0];
    bool del_rst = DbHelper::get_instance().handle_delete_friend(src_name,per_name);
    if(del_rst)
    {
        Pack pack;
        pack.set_type(TYPE_DELETE_FRIEND_REPONSE);
        pack.set_content(DELETE_FRIEND_OK);
        pmyTcpSocket->write(pack.data());
    }
    return;
}

void CloudServe::handle_send_msg(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString strmsg = content[0];
    QString snd_name = content[1];
    Pack pack;
    pack.set_type(TYPE_SEND_MSG_REPONSE);
    pack.set_content(snd_name);
    pack.set_content(strmsg);
    for(int i=1;i<content.size();i++)
    {
        for(auto& potherTcpSocket : m_pmyTcpSockt_vec)
        {
            if(potherTcpSocket->get_name() == content[i])
            {
                potherTcpSocket->write(pack.data());
            }
        }
    }
    return;
}

void CloudServe::handle_create_dir(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString str_curPath = content[0];
    QString new_dirName = content[1];
    Pack pack;
    pack.set_type(TYPE_CREATE_DIR_REPONSE);
    QDir dir;
    bool ret = dir.exists(str_curPath);
    if(ret)
    {
        QString newPath = str_curPath + "/" + new_dirName;
        ret = dir.exists(newPath);
        if(ret)// 有同名文件夹
        {
            pack.set_content(CREATE_DIR_EXISTED);
        }
        else // 没有同名文件夹，创建新的文件夹
        {
            dir.mkdir(newPath);
            pack.set_content(CREATE_DIR_OK);
        }
    }
    else// 当前目录不存在
    {
        pack.set_content(CREATE_DIR_NOEXISTED);
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_flush_file(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString str_curDir = content[0];

    /*获得当前路径所有文件*/
    QDir dir(str_curDir);
    dir.setFilter(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System);
    QFileInfoList fileInfoList = dir.entryInfoList();

    /*将所有文件的名字和文件的类型发送给客户端*/
    Pack pack;
    pack.set_type(TYPE_FLUSH_FILE_REPONSE);
    for(int i=0; i < fileInfoList.size();i++)
    {
        pack.set_content(fileInfoList[i].fileName());
        if(fileInfoList[i].isDir())
        {
            pack.set_content("dir");
        }
        else
        {
            pack.set_content("reg");
        }
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_delete(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString str_curPath = content[0];
    QString del_dirName = content[1];
    QString newPath = str_curPath + "/" + del_dirName;
    QFileInfo fileInfo(newPath);

    bool ret = false;

    Pack pack;
    pack.set_type(TYPE_DELETE_DIR_REPONSE);
    if(fileInfo.isDir())
    {
        /*删除路径*/
        ret = delete_dir(newPath);
        //        QDir dir;
        //        ret = dir.rmpath(newPath);
    }
    else if(fileInfo.isFile())
    {
        /*不是路径*/
        //ret = fileInfo.dir().remove(fileInfo.fileName());
        ret = fileInfo.dir().remove(fileInfo.absoluteFilePath());
    }
    if(ret)
    {
        pack.set_content(DELETE_DIR_OK);
    }
    else
    {
        pack.set_content(DELETE_NOT_DIR);
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_enter_dir(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString str_curDir = content[0];
    QString str_etName = content[1];
    QString newPath = str_curDir + "/" + str_etName;

    QFileInfo fileInfo(newPath);
    Pack pack;
    pack.set_type(TYPE_ENTER_DIR_REPONSE);
    if(fileInfo.isDir())
    {
        pack.set_content(ENTER_DIR_IS_DIR);
    }
    else
    {
        pack.set_content(ENTER_DIR_NOT_DIR);
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_rename_dir(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString str_curDir = content[0];
    QString oldName = content[1];
    QString newName = content[2];

    QString oldPath = str_curDir + "/" + oldName;
    QString newPath = str_curDir + "/" + newName;

    QDir dir;
    Pack pack;
    pack.set_type(TYPE_RENAME_DIR_REPONSE);

    if(dir.rename(oldPath,newPath))
    {
        pack.set_content(RENAME_OK);
    }
    else
    {
        pack.set_content(RENAME_ERROR);
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_upload_file(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString str_curDir = content[0];
    QString fileName = content[1];
    qint64 fileSize = content[2].toLongLong();
    QString str_md5 = content[3];

    Pack pack;
    pack.set_type(TYPE_UPLOAD_FILE_REPONSE);

    /*拼接文件路径*/
    //QString filePath = str_curDir + "/" + fileName;
    QString filePath = "./file/" + fileName;

    /*使用全局文件描述符*/
    pmyTcpSocket->get_uploadFile()->setFileName(filePath); // 设定文件路径
    //m_uploadFile.setFileName(filePath);

    /*用户上传文件时，先从数据库中查询是否有相同文件*/
    QString queryFileName = DbHelper::get_instance().handle_file_isExited(str_md5.toStdString().c_str());

    if(!queryFileName.isEmpty())
    {
        /*如果数据库中有相同md5值的文件存在，则在用户当前目录下直接创建链接文件，客户端无需向服务器发送文件内容*/
        QString fileLnkName = QString("%1%2").arg(fileName).arg(".lnk");  //windows下需要家lnk文件扩展名

        QString linkFilePath = QString("%1/%2").arg(str_curDir).arg(fileLnkName);
        pmyTcpSocket->get_uploadFile()->link(linkFilePath);
        pack.set_content(UPLOAD_FILE_COMPLETE);
    }
    else
    {
        //if(pmyTcpSocket->get_uploadFile()->open(QIODevice::Append))
        if(pmyTcpSocket->get_uploadFile()->open(QIODevice::Append)) // 只写方式打开文件
        {
            pack.set_content(UPLOAD_FILE_OPEN_OK);
            //pmyTcpSocket->set_uploading(true); // 如果打开成功，将正在上传状态改为真
            pmyTcpSocket->setUploadFileSize(fileSize); // 设定文件大小
            pmyTcpSocket->setUploadFileRecived(0); // 设定已经接收的文件大小
            pmyTcpSocket->setUploadFileMd5(str_md5);  // 设定文件的MD5值
            pmyTcpSocket->setCurrentPath(str_curDir);  // 设定文件路径
        }
        else
        {
            pack.set_content(UPLOAD_FILE_OPEN_ERROR);
        }
    }
    pmyTcpSocket->write(pack.data());
    return;
}
void CloudServe::handle_upload_file_content(MyTcpSocket *pmyTcpSocket, const QStringList &content,const QByteArray& binary_data)
{
    /*将socket的内容全部读取出来*/
    pmyTcpSocket->get_uploadFile()->write(binary_data); // 写入文件
    pmyTcpSocket->setUploadFileRecived(pmyTcpSocket->getUploadFileRecived()+binary_data.size()); // 记录接收文件内容的大小

    /*当接收的大小 = 文件总大小则认为接收完成*/
    if(pmyTcpSocket->getUploadFileSize() <= pmyTcpSocket->getUploadFileRecived())
    {
        Pack pack;
        pack.set_type(TYPE_UPLOAD_FILE_REPONSE);
        /*文件上传完成*/
        pmyTcpSocket->set_uploading(false);
        pmyTcpSocket->get_uploadFile()->close();

        /*MD5校验*/
        pmyTcpSocket->get_uploadFile()->open(QFile::ReadOnly);
        QByteArray array = pmyTcpSocket->get_uploadFile()->readAll();
        pmyTcpSocket->get_uploadFile()->close();
        QByteArray md5_array =  QCryptographicHash::hash(array,QCryptographicHash::Md5);
        QString md5_str = md5_array.toHex().constData();

        if(md5_str == pmyTcpSocket->getUploadFileMd5())
        {
            /*把文件信息存储到数据库表中*/
            QString fileName = pmyTcpSocket->get_uploadFile()->fileName();
            QString db_md5 = pmyTcpSocket->getUploadFileMd5().toStdString().c_str();
            bool res = DbHelper::get_instance().handle_save_fileInfo(fileName.toStdString().c_str(),db_md5);
            int index = fileName.lastIndexOf("/");
            fileName.remove(0,index+1);
            /*发送上传完成消息*/
            if(res)
            {
                /*在当前目录创建对应文件的链接文件*/
                QString fileLnkName = fileName + ".lnk";
                QString linkFilePath = pmyTcpSocket->getCurrentPath() + "/" + fileLnkName;
                //createShortcut(pmyTcpSocket->get_uploadFile()->fileName(), linkFilePath);
                pmyTcpSocket->get_uploadFile()->link(linkFilePath);
                pack.set_content(UPLOAD_FILE_COMPLETE);
            }
            else
            {
                pack.set_content(UPLOAD_FILE_FAILD);
            }
        }
        else
        {
            pack.set_content(UPLOAD_FILE_CHECK_FAILD);
            pmyTcpSocket->get_uploadFile()->remove();
        }
        pmyTcpSocket->write(pack.data());
    }
    else
    {
        Pack pack;
        pack.set_type(TYPE_UPLOAD_FILE_CONTENT_REPONSE);
        pack.set_content(QString::number(pmyTcpSocket->getUploadFileRecived()));
        pmyTcpSocket->write(pack.data());
    }
    return;
}
#if 0
void CloudServe::handle_upload_file_content(MyTcpSocket *pmyTcpSocket, const QStringList &content,const QByteArray& binary_data)
{
    Pack pack;
    pack.set_type(TYPE_UPLOAD_FILE_REPONSE);
    /*将socket的内容全部读取出来*/
    pmyTcpSocket->get_uploadFile()->write(binary_data); // 写入文件
    pmyTcpSocket->setUploadFileRecived(pmyTcpSocket->getUploadFileRecived()+binary_data.size()); // 记录接收文件内容的大小

    /*当接收的大小 = 文件总大小则认为接收完成*/
    if(pmyTcpSocket->getUploadFileSize() <= pmyTcpSocket->getUploadFileRecived())
    {
        /*文件上传完成*/
        pmyTcpSocket->set_uploading(false);
        pmyTcpSocket->get_uploadFile()->close();

        /*MD5校验*/
        pmyTcpSocket->get_uploadFile()->open(QFile::ReadOnly);
        QByteArray array = pmyTcpSocket->get_uploadFile()->readAll();
        pmyTcpSocket->get_uploadFile()->close();
        QByteArray md5_array =  QCryptographicHash::hash(array,QCryptographicHash::Md5);
        QString md5_str = md5_array.toHex().constData();

        if(md5_str == pmyTcpSocket->getUploadFileMd5())
        {
            /*把文件信息存储到数据库表中*/
            QString fileName = pmyTcpSocket->get_uploadFile()->fileName();
            QString db_md5 = pmyTcpSocket->getUploadFileMd5().toStdString().c_str();
            bool res = DbHelper::get_instance().handle_save_fileInfo(fileName.toStdString().c_str(),db_md5);
            int index = fileName.lastIndexOf("/");
            fileName.remove(0,index+1);
            /*发送上传完成消息*/
            if(res)
            {
                QString fileLnkName = fileName + ".lnk";
                QString linkFilePath = pmyTcpSocket->getCurrentPath() + "/" + fileLnkName;
                //createShortcut(pmyTcpSocket->get_uploadFile()->fileName(), linkFilePath);
                pmyTcpSocket->get_uploadFile()->link(linkFilePath);
                pack.set_content(UPLOAD_FILE_COMPLETE);
            }
            else
            {
                pack.set_content(UPLOAD_FILE_FAILD);
            }
        }
        else
        {
            pack.set_content(UPLOAD_FILE_CHECK_FAILD);
            pmyTcpSocket->get_uploadFile()->remove();
        }
        pmyTcpSocket->write(pack.data());
    }
}
#endif
void CloudServe::handle_download_file(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString str_fileName = content[0];
    QString str_curDir = content[1];

    QString filePath = str_curDir + "/" + str_fileName;
    QFileInfo fileInfo(filePath);
    if(fileInfo.isDir())
    {
        Pack pack;
        pack.set_type(TYPE_DOWNLOAD_FILE_REPONSE);
        pack.set_content(DOWN_LOAD_FILE_ISDIR);
        pmyTcpSocket->write(pack.data());
    }
    else
    {
        QFile file(filePath);
        QString fileTargetPath = file.symLinkTarget();
        pmyTcpSocket->setDownLoadFilePath(fileTargetPath); // 保存真实路径

        /*将文件名文件大小发送给客户端*/
        QFileInfo fileInfo(fileTargetPath);
        QString fileName = fileInfo.fileName();
        qint64 fileSize = fileInfo.size();

        /*获取文件md5值*/
        QFile theFile(fileTargetPath);
        theFile.open(QFile::ReadOnly);
        QByteArray array = theFile.readAll();
        theFile.close();
        QByteArray md5_array =  QCryptographicHash::hash(array,QCryptographicHash::Md5);
        QString md5_str = md5_array.toHex().constData();

        Pack pack;
        pack.set_type(TYPE_DOWNLOAD_FILE_REPONSE);
        pack.set_content(fileName);
        pack.set_content(QString::number(fileSize));
        pack.set_content(md5_str);
        pmyTcpSocket->write(pack.data());
    }
    return;
}

void CloudServe::handle_download_file_content(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    quint64 offset = content[0].toLongLong();

    Pack pack;
    pack.set_type(TYPE_DOWNLOAD_FILE_CONTENT_REPONSE);

    QByteArray buff;
    QFile file(pmyTcpSocket->getDownLoadFilePath());
    if(file.open(QIODevice::ReadOnly))
    {
        pack.set_content("0");
        file.seek(offset);
        buff = file.read(4096);
        pack.set_binary_size(buff.size());
        pmyTcpSocket->get_tcpsocket()->write(pack.data() + buff);
    }
    else
    {
        pack.set_content("-1");
        pmyTcpSocket->get_tcpsocket()->write(pack.data());
    }
    return;
}

void CloudServe::handle_move_file(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString srcDir = content[0];
    QString fileName = content[1];
    QString desDir = content[2];

    QString srcPath = srcDir + "/" + fileName;
    QString desPath = desDir + "/" + fileName;

    QFile file;
    bool rst = file.rename(srcPath,desPath);
    Pack pack;
    pack.set_type(TYPE_MOVE_FILE_REPONSE);
    if(rst)
    {
        pack.set_content(MOVE_FILE_OK);
    }
    else
    {
        pack.set_content(MOVE_FILE_FAILED);
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_share_online_friends(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QStringList users = DbHelper::get_instance().get_online_friends(pmyTcpSocket->get_name());
    Pack pack;
    pack.set_type(TYPE_SHARE_ONLINE_FRIENDS_REPONSE);
    for(auto& name : users)
    {
        pack.set_content(name);
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_share_file(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString srcName = content[0];
    QString filePath = content[1];

    Pack pack;
    pack.set_type(TYPE_SHARE_FILE_REQUEST);
    pack.set_content(srcName);
    pack.set_content(filePath);
    /*循环给需要分享的好友发送分享者用户名和分享的文件路径*/
    for(int i=2; i<content.size();i++)
    {
        for(auto& potherTcpSocket : m_pmyTcpSockt_vec)
        {
            if(potherTcpSocket->get_name() == content[i])
            {
                potherTcpSocket->write(pack.data());
            }
        }
    }

    /*告知文件分享者分享成功*/
    pack.set_type(TYPE_SHARE_FILE_REPONSE);
    pack.clear();
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_share_file_agree(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString userName = content[0];
    QString filePath = content[1];

    QFileInfo fileInfo(filePath);

    QString desPath = "./" + userName + "/" + fileInfo.fileName();


    if(fileInfo.isDir())
    {
        copy_dir(filePath,desPath);
    }
    else
    {
        QFile file(filePath);
        QString fileName = file.symLinkTarget();
        QFile fileTure(fileName);
        fileTure.link(desPath);
        // QFile::copy(filePath,desPath);
    }
    return;
}

void CloudServe::handle_share_file_link(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString filePath = content[0];
    int timeOut = content[1].toInt();

    QString link = QUuid::createUuid().toString();
    link.remove("{").remove("}").remove("-");
    qDebug() << link;

    QDateTime dataTime = QDateTime::currentDateTime();
    QString createTime = dataTime.toString("yyyy-MM-dd hh:mm:ss");

    Pack pack;
    pack.set_type(TYPE_SHARE_FILE_LINK_REPONSE);
    bool rst =  DbHelper::get_instance().handle_create_share_link(filePath,link,createTime,timeOut);
    if(rst)
    {
        pack.set_content("success");
        pack.set_content(link);
    }
    else
    {
        pack.set_content("faild");
    }
    pmyTcpSocket->write(pack.data());
    return;
}

void CloudServe::handle_get_file_from_link(MyTcpSocket *pmyTcpSocket, const QStringList &content)
{
    QString userName = content[0];
    QString link = content[1];
    char filePath[128] = {0};
    QString rst = DbHelper::get_instance().get_file_by_link(link,filePath);

    Pack pack;
    pack.set_type(TYPE_GET_FILE_FROM_LINK_REPONSE);

    if(rst == GET_FILE_FROM_LINK_OK)
    {
        QFileInfo fileInfo(filePath);
        if(fileInfo.exists())
        {
            QString desPath = "./" + userName + "/" + fileInfo.fileName();
            if(fileInfo.isDir())
            {
                copy_dir(filePath,desPath);
            }
            else
            {
                QFile file(filePath);
                QString fileName =  file.symLinkTarget();
                QFile fileTrue(fileName);
                fileTrue.link(desPath);
            }
        }
        else
        {
            rst = GET_FILE_FROM_LINK_FILENOTEXISTED;
        }
    }

    pack.set_content(rst);
    pmyTcpSocket->write(pack.data());
    return;
}

// 根据QTcpSocket在容器中查找MyTcpSocket并返回MyTcpSocket
MyTcpSocket *CloudServe::find_pmyTcpSocket(QTcpSocket *ptcpsocket)
{
    for(auto potherTcpSocket : m_pmyTcpSockt_vec)
    {
        if(potherTcpSocket->get_tcpsocket() == ptcpsocket)
        {
            return potherTcpSocket;
        }
    }
    return nullptr;
}

bool CloudServe::delete_dir(QString path)
{
    if(path.isEmpty())
    {
        return false;
    }
    QDir dir(path);
    if(!dir.exists())
        return true;
    dir.setFilter(QDir::AllEntries|QDir::NoDotAndDotDot);
    QFileInfoList file_list = dir.entryInfoList();
    foreach (QFileInfo file, file_list)
    {
        if(file.isFile())
            file.dir().remove(file.fileName());
        else
            delete_dir(file.absoluteFilePath()); // 如果是目录则递归删除
    }
    return dir.rmdir(dir.absolutePath());
}

bool CloudServe::copy_dir(QString srcDir, QString desDir)
{
    QDir dir;
    dir.mkdir(desDir);

    dir.setPath(srcDir);
    dir.setFilter(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System);

    QFileInfoList fileInfoList = dir.entryInfoList();

    QString srcTmp,desTmp;

    for(int i=0; i<fileInfoList.size();i++)
    {
        // 如果下层是目录则递归调用
        if(fileInfoList[i].isDir())
        {
            srcTmp = srcDir + "/" + fileInfoList[i].fileName();
            desTmp = desTmp + "/" + fileInfoList[i].fileName();
            copy_dir(srcTmp,desTmp);
        }
        else// 如果是文件则直接复制
        {
            srcTmp = srcDir + "/" + fileInfoList[i].fileName();
            desTmp = desTmp + "/" + fileInfoList[i].fileName();
            QFile file(srcTmp);
            QString fileName = file.symLinkTarget();
            QFile fileTure(fileName);
            fileTure.link(desTmp);
            // QFile::copy(srcTmp,desTmp);
        }
    }
    return true;
}
