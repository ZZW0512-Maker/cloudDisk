#ifndef FILEWINDOW_H
#define FILEWINDOW_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTimer>
#include <QThread>
#include "sharewindow.h"
#include "uploadfilethread.h"

namespace Ui {
class FileWindow;
}

class FileWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FileWindow(QWidget *parent = nullptr);
    ~FileWindow();

public:
    void fileWindow_init();

public:
    void handle_create_dir_reponse(const QStringList& content); // 处理创建文件夹消息回复
    void handle_flush_file_reponse(const QStringList& content); // 处理刷新文件消息回复
    void handle_delete_dir_reponse(const QStringList& content); // 处理删除文件夹消息回复
    void handle_enter_dir_reponse(const QStringList& content); // 处理进入文件夹消息回复
    void handle_rename_dir_reponse(const QStringList& content); // 处理重命名文件夹消息回复
    void handle_upload_file_reponse(const QStringList& content); // 处理上传文件消息回复
    void handle_upload_file_content_reponse(const QStringList& content); // 处理上传文件内容消息回复
    void handle_download_file_reponse(const QStringList& content); // 处理下载文件消息回复
    void download_file_content_request();                           // 发送下载文件内容请求
    void handle_download_file_content_reponse(const QStringList& content,const QByteArray& buff); // 处理下载文件内容消息回复
    void handle_move_file_reponse(const QStringList& content);// 处理移动文件消息回复
    void handle_online_friend_reponse(const QStringList& content);// 处理获取在线好友消息回复
    void handle_share_file_reponse(const QStringList& content);// 处理分享文件消息回复
    void handle_share_file_request(const QStringList& content);// 处理分享文件消息请求
    void handle_share_file_link_reponse(const QStringList& content);// 处理分享文件链接消息请求
    void handle_get_file_from_link_reponse(const QStringList& content);// 处理从链接获取文件消息回复

private slots:
    void on_bt_createDir_clicked();

    void on_bt_flushfile_clicked();

    void on_bt_deletedir_clicked();

    void on_bt_back_clicked();

    void on_bt_renameDir_clicked();

    void on_bt_uploadfile_clicked();

    void on_bt_download_clicked();

    void on_bt_movefile_clicked();

    void on_bt_sharefile_clicked();

    void on_bt_sharelink_clicked();

    void on_bt_getfile_clicked();

public slots:
    void enter_dir(QModelIndex index);
    // void enter_dir(QListWidgetItem *item);
    // void uploadFileContent();
    void send_file_content();


private:
    Ui::FileWindow *ui;
    QString m_enterDirName;
    QTimer m_timer;
    QString m_uploadFilePath;
    UpLoadFileThread *m_uploadThread;
    qint64 m_uploadFileOffset;
    QString m_saveFilePath;
    QString m_downLoadFileName;
    qint64 m_downLoadFileSize;
    QString m_downLoadFileMd5;
    qint64 m_downLoadFileOffset;
    QString m_moveSrcFileDir;
    QString m_moveDesFileDir;
    QString m_moveSrcFileName;
    ShareWindow *sharewd;
    QString m_shareFileName;
};

#endif // FILEWINDOW_H
