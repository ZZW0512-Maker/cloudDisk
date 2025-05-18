#include "filewindow.h"
#include "ui_filewindow.h"
#include "cloudclient.h"
#include "pack.h"
#include <QThread>
#include <QInputDialog>
#include <QMessageBox>
#include <QIcon>
#include <QPixmap>
#include <QModelIndex>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QClipboard>

FileWindow::FileWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileWindow)
  , m_enterDirName()
  , m_timer()
  , m_uploadFilePath()
  , m_uploadThread(nullptr)
  , m_uploadFileOffset()
  , m_saveFilePath()
  , m_downLoadFileName()
  , m_downLoadFileSize()
  , m_downLoadFileMd5()
  , m_downLoadFileOffset()
  , m_moveSrcFileDir()
  , m_moveDesFileDir()
  , m_moveSrcFileName()
  , sharewd(new ShareWindow)
  , m_shareFileName()
{
    ui->setupUi(this);
    connect(ui->file_list,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(enter_dir(QModelIndex))); // 双击QListWidgetItem项响应槽函数
    //connect(ui->file_list,&QListWidget::itemDoubleClicked,this,&FileWindow::enter_dir);
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(uploadFileContent())); // 定时器终止响应槽函数
}

FileWindow::~FileWindow()
{
    delete ui;
}

void FileWindow::fileWindow_init()
{
    ui->bt_flushfile->click();
}

void FileWindow::handle_create_dir_reponse(const QStringList &content)
{
    if(CREATE_DIR_OK == content[0])
    {
        ui->bt_flushfile->click();
    }
    else if(CREATE_DIR_NOEXISTED == content[0])
    {
        QMessageBox::information(this,"创建文件夹","当前路径不存在");
    }
    else if(CREATE_DIR_EXISTED == content[0])
    {
        QMessageBox::information(this,"创建文件夹","已有同名文件夹");
    }
    return;
}

void FileWindow::handle_flush_file_reponse(const QStringList &content)
{
    QListWidgetItem *pitem = nullptr;
    while(ui->file_list->count())
    {
        pitem = ui->file_list->item(ui->file_list->count()-1);
        ui->file_list->removeItemWidget(pitem);
        delete pitem;
    }
    for(int i=0 ; i < content.size();i=i+2)
    {
        QString fileName = content[i];
        QString fileType = content[i+1];
        QListWidgetItem *item = new QListWidgetItem;
        if(fileType == "dir")
        {
            item->setIcon(QIcon(QPixmap(":/dir.jpg")));
        }
        else if(fileType == "reg")
        {
            item->setIcon(QIcon(QPixmap(":/reg.jpg")));
        }
        item->setText(fileName);
        ui->file_list->addItem(item);
    }
    return;
}

void FileWindow::handle_delete_dir_reponse(const QStringList &content)
{
    if(content[0] == DELETE_DIR_OK)
    {
        // QMessageBox::information(this,"删除","删除文件夹成功");
        ui->bt_flushfile->click();
    }
    else
    {
        QMessageBox::warning(this,"删除","选中的为非文件夹");
    }
    return;
}

void FileWindow::handle_enter_dir_reponse(const QStringList &content)
{
    if(content[0] == ENTER_DIR_IS_DIR)
    {
        QString newPath = CloudClient::get_instance().get_current_dir() + "/" + m_enterDirName;
        CloudClient::get_instance().set_current_dir(newPath);
        ui->bt_flushfile->click();
        m_enterDirName.clear();
        ui->lb_curdir->setText(newPath);
    }
    else
    {
        QMessageBox::information(this,"进入文件夹","选中的非文件夹");
    }
    return;
}

void FileWindow::handle_rename_dir_reponse(const QStringList &content)
{
    if(content[0] == RENAME_OK)
    {
        ui->bt_flushfile->click();
    }
    else
    {
        QMessageBox::warning(this,"重命名文件夹","重命名失败");
    }
    return;
}

void FileWindow::handle_upload_file_reponse(const QStringList &content)
{
    if(content[0] == UPLOAD_FILE_OPEN_OK)
    {
        //QMessageBox::information(this,"上传文件","上传文件打开成功");
        // m_timer.start(100); // 启动定时，定时结束后开始上传
        //        m_uploadThread = new UpLoadFileThread(m_uploadFilePath);
        //        m_uploadThread->start();
        m_uploadFileOffset = 0;
        send_file_content();
    }
    else if(content[0] == UPLOAD_FILE_COMPLETE)
    {
        QMessageBox::information(this,"上传文件","文件上传成功");
        ui->bt_flushfile->click();
        //        delete m_uploadThread;
        //        m_uploadThread = nullptr;
    }
    else if(content[0] == UPLOAD_FILE_OPEN_ERROR)
    {
        QMessageBox::information(this,"上传文件","上传文件打开失败");
    }
    else if(content[0] == UPLOAD_FILE_CHECK_FAILD)
    {
        QMessageBox::information(this,"上传文件","上传文件校验失败");
    }
    else if(content[0] == UPLOAD_FILE_EXITED_OK)
    {
        QMessageBox::information(this,"上传文件","上传文件已存在");
    }
    return;
}

void FileWindow::handle_upload_file_content_reponse(const QStringList &content)
{
    m_uploadFileOffset = content[0].toLongLong();
    send_file_content();
}

void FileWindow::handle_download_file_reponse(const QStringList &content)
{
    if(content[0] == DOWN_LOAD_FILE_ISDIR)
    {
        QMessageBox::warning(this,"下载文件","文件夹不能下载");
        return;
    }
    QString fileName = content[0];
    quint64 fileSize = content[1].toLongLong();
    QString strMd5 = content[2];

    m_downLoadFileName = fileName;
    m_downLoadFileSize = fileSize;
    m_downLoadFileMd5 = strMd5;
    m_saveFilePath = m_saveFilePath + "/" + m_downLoadFileName;

    QFile file(m_saveFilePath);
    if(file.exists())
    {
        int res = QMessageBox::information(this,"下载文件","文件已存在，是否覆盖",QMessageBox::Yes,QMessageBox::No);
        if(res == QMessageBox::Yes)
        {
            QFile file(m_saveFilePath);
            file.open(QIODevice::WriteOnly);
            file.close();//清除之前文件内容
            /*获取文件内容*/
            m_downLoadFileOffset = 0;
            download_file_content_request();
        }
    }
    else
    {
        m_downLoadFileOffset = 0;
        download_file_content_request();
    }
    return;
}

void FileWindow::download_file_content_request()
{
    Pack pack;
    pack.set_type(TYPE_DOWNLOAD_FILE_CONTENT_REQUEST);
    pack.set_content(QString::number(m_downLoadFileOffset));
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    return;
}

void FileWindow::handle_download_file_content_reponse(const QStringList &content, const QByteArray &buff)
{
    if(content[0] == -1)
    {
        QMessageBox::information(this,"下载文件","下载文件失败");
        QFile file(m_saveFilePath);
        file.remove();
    }
    else
    {
        QFile file(m_saveFilePath);
        if(file.open(QIODevice::Append))
        {
            file.seek(m_downLoadFileOffset);
            file.write(buff);
            file.close();
            m_downLoadFileOffset += buff.size();
            if(m_downLoadFileOffset >= m_downLoadFileSize)
            {
                /*md5校验*/
                QFile theFile(m_saveFilePath);
                theFile.open(QFile::ReadOnly);
                QByteArray array = theFile.readAll();
                theFile.close();
                QByteArray md5_array =  QCryptographicHash::hash(array,QCryptographicHash::Md5);
                QString md5_str = md5_array.toHex().constData();
                if(md5_str == m_downLoadFileMd5)
                {
                    QMessageBox::information(this,"下载文件","文件下载成功");
                }
                else
                {
                    QMessageBox::information(this,"下载文件","文件下载失败，md5值不正确");
                }
            }
            else
            {
                download_file_content_request(); // 发送偏移量继续下载
            }
        }
    }
}

void FileWindow::handle_move_file_reponse(const QStringList& content)
{
    if(content[0] == MOVE_FILE_OK)
    {
        ui->bt_flushfile->click();
    }
    else
    {
        QMessageBox::information(this,"移动文件","文件移动失败");
    }
    return;
}

void FileWindow::handle_online_friend_reponse(const QStringList &content)
{
    sharewd->handle_online_friends_reponse(content);
    return;
}

void FileWindow::handle_share_file_reponse(const QStringList &content)
{
    QMessageBox::information(this,"分享文件","分享成功");
    return;
}

void FileWindow::handle_share_file_request(const QStringList &content)
{
    QString srcName = content[0];
    QString filePath = content[1];
    QString localName = CloudClient::get_instance().get_name();
    int rst = QMessageBox::information(this,"分享文件",QString("%1 向你分享文件%2，是否同意？").arg(srcName).arg(filePath),QMessageBox::Yes,QMessageBox::No);
    if(QMessageBox::Yes == rst)
    {
        /*如果用户同意，则将同意请求发送给服务端，发送的内容包括当前用户名和文件路径*/
        Pack pack;
        pack.set_type(TYPE_SHARE_FILE_AGREE);
        pack.set_content(localName);
        pack.set_content(filePath);
        CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    }
    return;
}

void FileWindow::handle_share_file_link_reponse(const QStringList &content)
{
    if(content[0] == "success")
    {
        //QMessageBox::information(this,"分享文件链接",content[1]);
        // 创建消息框
        QMessageBox msgBox;
        msgBox.setWindowTitle("分享文件链接");
        msgBox.setText(content[1]);

        // 添加标准按钮（如确定）
        QPushButton *okButton = msgBox.addButton(QMessageBox::Ok);
        okButton->setText("确定");
        // 添加自定义“复制”按钮
        QPushButton *copyButton = msgBox.addButton("复制", QMessageBox::ActionRole);

        // 显示消息框
        msgBox.exec();

        // 检测是否点击了“复制”按钮
        if (msgBox.clickedButton() == copyButton)
        {
            // 将消息框内容复制到剪贴板
            QString contentToCopy = msgBox.text(); // 获取弹窗中的文本内容
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(contentToCopy); // 将内容复制到剪贴板
        }
    }
    else
    {
        QMessageBox::information(this,"分享文件链接","创建链接失败");
    }
    return;
}

void FileWindow::handle_get_file_from_link_reponse(const QStringList &content)
{
    if(content[0] == GET_FILE_FROM_LINK_OK)
    {
        on_bt_flushfile_clicked();
        QMessageBox::information(this,"获取文件","获取成功");
    }
    else if(content[0] == GET_FILE_FROM_LINK_NOTEXISTED)
    {
        QMessageBox::information(this,"获取文件","链接不存在");
    }
    else if(content[0] == GET_FILE_FROM_LINK_TIMEOUT)
    {
        QMessageBox::information(this,"获取文件","链接失效");
    }
    else if(content[0] == GET_FILE_FROM_LINK_FILENOTEXISTED)
    {
        QMessageBox::information(this,"获取文件","文件不存在");
    }
    return;
}

void FileWindow::on_bt_createDir_clicked()
{
    QString str_curDir = CloudClient::get_instance().get_current_dir();
    QString new_dirName = QInputDialog::getText(this,"创建文件夹","文件夹名:");
    if(!new_dirName.isEmpty())
    {
        if(new_dirName.size()>32)
        {
            QMessageBox::information(this,"创建文件夹","文件夹名称不能超过32个字节");
            return;
        }
        Pack pack;
        pack.set_type(TYPE_CREATE_DIR_REQUEST);
        pack.set_content(str_curDir);
        pack.set_content(new_dirName);
        CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    }
    else
    {
        QMessageBox::information(this,"创建文件夹","文件夹名不能为空");
    }
    return;
}

void FileWindow::on_bt_flushfile_clicked()
{
    QString str_curDir = CloudClient::get_instance().get_current_dir();
    Pack pack;
    pack.set_type(TYPE_FLUSH_FILE_REQUEST);
    pack.set_content(str_curDir);
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    ui->lb_curdir->setText(str_curDir);
    return;
}

void FileWindow::on_bt_deletedir_clicked()
{
    QListWidgetItem *item = ui->file_list->currentItem();
    if(item == nullptr)
    {
        return;
    }
    QString str_curDir = CloudClient::get_instance().get_current_dir();
    QString del_dirName = item->text();
    Pack pack;
    pack.set_type(TYPE_DELETE_DIR_REQUEST);
    pack.set_content(str_curDir);
    pack.set_content(del_dirName);
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    return;
}

//void FileWindow::enter_dir(QListWidgetItem *item)
//{
//    QString str_curDir = CloudClient::get_instance().get_current_dir();
//    QString str_etName = item->text();

//    m_enterDirName.clear();
//    m_enterDirName = str_etName;

//    Pack pack;
//    pack.set_type(TYPE_ENTER_DIR_REQUEST);
//    pack.set_content(str_curDir);
//    pack.set_content(str_etName);
//    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
//    return;
//}

void FileWindow::enter_dir(QModelIndex index)
{
    QString str_curDir = CloudClient::get_instance().get_current_dir();
    QString str_etName = index.data().toString();

    m_enterDirName.clear();
    m_enterDirName = str_etName;

    Pack pack;
    pack.set_type(TYPE_ENTER_DIR_REQUEST);
    pack.set_content(str_curDir);
    pack.set_content(str_etName);
    CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    return;
}

void FileWindow::send_file_content()
{
    // m_timer.stop();
    QFile file(m_uploadFilePath);
    if(file.open(QIODevice::ReadOnly))
    {
        Pack pack;
        pack.set_type(TYPE_UPLOAD_FILE_CONTENT_REQUEST);
        QByteArray buff;
        buff.clear();
        file.seek(m_uploadFileOffset);
        buff = file.read(4096);
        if(buff.size()>0)
        {
            m_uploadFileOffset += buff.size();
            pack.set_binary_size(buff.size());
            CloudClient::get_instance().get_tcpSocket()->write(pack.data() + buff);
        }
        file.close();
    }
    else
    {
        QMessageBox::information(this,"上传文件","文件打开失败");
    }
    return;
}

#if 0
void FileWindow::uploadFileContent()
{
    // m_timer.stop();
    QFile file(m_uploadFilePath);
    if(file.open(QIODevice::ReadOnly))
    {
        Pack pack;
        pack.set_type(TYPE_UPLOAD_FILE_CONTENT_REQUEST);


        //char buff[4096];
        QByteArray buff;
        qint64 ret = 0;
        /*循环读取，发送*/
        while(true)
        {
            //memset(buff,0,sizeof(buff));
            //ret = file.read(buff,4096);
            buff.clear();
            buff = file.read(4096);
            //            if(ret > 0)
            if(buff.size()>0)
            {
                pack.clear();
                pack.set_binary_size(buff.size());
                CloudClient::get_instance().get_tcpSocket()->write(pack.data() + buff);
                //CloudClient::get_instance().get_tcpSocket()->write(buff);
                //CloudClient::get_instance().get_tcpSocket()->write((char*)buff,ret);
            }
            //else if(ret == 0)//读取完成
            else if(buff.size() == 0)
            {
                break;
            }
            else
            {
                QMessageBox::information(this,"上传文件","文件读取错误");
            }
        }
        file.close();
    }
    else
    {
        QMessageBox::information(this,"上传文件","文件打开失败");
    }
    return;
}
#endif

void FileWindow::on_bt_back_clicked()
{

    QString str_curDir = CloudClient::get_instance().get_current_dir();
    if(str_curDir == CloudClient::get_instance().get_name())   /*已经是顶层目录*/
    {
        QMessageBox::information(this,"返回上一层","已经是顶层目录");
        return;
    }
    int index = str_curDir.lastIndexOf('/');
    str_curDir.remove(index,str_curDir.size()-index);
    CloudClient::get_instance().set_current_dir(str_curDir);
    ui->bt_flushfile->click();

    ui->lb_curdir->setText(str_curDir);
    return;
}

void FileWindow::on_bt_renameDir_clicked()
{
    QString str_curDir = CloudClient::get_instance().get_current_dir();
    QListWidgetItem *item = ui->file_list->currentItem();
    if(item != nullptr)
    {
        if(item->text().size() > 32)
        {
            QMessageBox::information(this,"重命名文件夹","新的文件夹名长度不得超过32字节");
            return;
        }
        QString oldName = item->text(); // 原文件名
        QString newName = QInputDialog::getText(this,"文件夹重命名","新的文件夹名:"); // 新文件夹名
        Pack pack;
        pack.set_type(TYPE_RENAME_DIR_REQUEST);
        pack.set_content(str_curDir);
        pack.set_content(oldName);
        pack.set_content(newName);
        CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    }
    return;
}

void FileWindow::on_bt_uploadfile_clicked()
{
    QString str_curDir = CloudClient::get_instance().get_current_dir();
    QString filePath = QFileDialog::getOpenFileName();
    if(!filePath.isEmpty())
    {
        /*记录文件名*/
        m_uploadFilePath.clear();
        m_uploadFilePath = filePath;

        /*将文件路径文件名文件大小发送给服务端*/
        QString str_curDir = CloudClient::get_instance().get_current_dir();
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        qint64 fileSize = fileInfo.size();

        /*md5校验*/
        QFile theFile(filePath);
        theFile.open(QFile::ReadOnly);
        QByteArray array = theFile.readAll();
        theFile.close();
        QByteArray md5_array =  QCryptographicHash::hash(array,QCryptographicHash::Md5);
        QString md5_str = md5_array.toHex().constData();

        Pack pack;
        pack.set_type(TYPE_UPLOAD_FILE_REQUEST);
        pack.set_content(str_curDir);
        pack.set_content(fileName);
        pack.set_content(QString::number(fileSize));
        pack.set_content(md5_str);
        CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    }
    else
    {
        //        QMessageBox::information(this,"上传文件","请选择需要上传的文件");
    }
    return;
}

void FileWindow::on_bt_download_clicked()
{
    QListWidgetItem *item = ui->file_list->currentItem();
    if(item == nullptr)
    {
        QMessageBox::information(this,"下载文件","请选择需要下载的文件");
    }
    else
    {
        QString saveFilePath = QFileDialog::getExistingDirectory();
        if(!saveFilePath.isEmpty())
        {
            m_saveFilePath = saveFilePath;
            QString fileName = item->text();
            QString curDir = CloudClient::get_instance().get_current_dir();
            Pack pack;
            pack.set_type(TYPE_DOWNLOAD_FILE_REQUEST);
            pack.set_content(fileName);
            pack.set_content(curDir);
            CloudClient::get_instance().get_tcpSocket()->write(pack.data());
        }
    }
    return;
}

void FileWindow::on_bt_movefile_clicked()
{
    QString bt_moveFileText = ui->bt_movefile->text();
    if(bt_moveFileText == "移动文件")
    {
        QListWidgetItem *item = ui->file_list->currentItem();
        if(item != nullptr)
        {
            QString fileName = item->text();
            QString curDir = CloudClient::get_instance().get_current_dir();

            m_moveSrcFileName = fileName;
            m_moveSrcFileDir = curDir;

            ui->bt_movefile->setText("确认移动");
        }
        else
        {
            QMessageBox::information(this,"移动文件","请选择要移动的文件");
        }
    }
    else
    {
        ui->bt_movefile->setText("移动文件");
        QString str_curDir = CloudClient::get_instance().get_current_dir();
        m_moveDesFileDir = str_curDir;
        if(m_moveDesFileDir == m_moveSrcFileDir)
        {
            QMessageBox::information(this,"移动文件","目标文件夹为当前文件夹");
        }
        else
        {
            Pack pack;
            pack.set_type(TYPE_MOVE_FILE_REQUEST);
            pack.set_content(m_moveSrcFileDir);
            pack.set_content(m_moveSrcFileName);
            pack.set_content(m_moveDesFileDir);
            CloudClient::get_instance().get_tcpSocket()->write(pack.data());
        }
    }
    return;
}

void FileWindow::on_bt_sharefile_clicked()
{
    QListWidgetItem *item = ui->file_list->currentItem();
    if(item != nullptr)
    {
        m_shareFileName = item->text();
        sharewd->show_friend_list(m_shareFileName);
        sharewd->show();
    }

}

void FileWindow::on_bt_sharelink_clicked()
{
    QListWidgetItem *item = ui->file_list->currentItem();
    if(item != nullptr)
    {
        QString str_curDir = CloudClient::get_instance().get_current_dir();
        QString fileName = item->text();
        str_curDir = str_curDir + "/" + fileName;
        int time = QInputDialog::getInt(this,"分享文件","文件时效:");
        if(time > 0)
        {
            Pack pack;
            pack.set_type(TYPE_SHARE_FILE_LINK_REQUEST);
            pack.set_content(str_curDir);
            pack.set_content(QString::number(time));
            CloudClient::get_instance().get_tcpSocket()->write(pack.data());
        }
        else
        {
            QMessageBox::information(this,"分享文件","链接时效小于等于0，等于0则链接不会失效");
        }
    }
    else
    {
        QMessageBox::information(this,"分享文件","请选中需要分享的文件");
    }
    return;
}

void FileWindow::on_bt_getfile_clicked()
{
    QString link = QInputDialog::getText(this,"获取文件","文件链接:");

    if(!link.isEmpty())
    {
        QString userName = CloudClient::get_instance().get_name();
        Pack pack;
        pack.set_type(TYPE_GET_FILE_FROM_LINK_REQUEST);
        pack.set_content(userName);
        pack.set_content(link);
        CloudClient::get_instance().get_tcpSocket()->write(pack.data());
    }
    return;
}
