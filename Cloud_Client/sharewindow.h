#ifndef SHAREWINDOW_H
#define SHAREWINDOW_H

#include <QWidget>
#include "cloudclient.h"

namespace Ui {
class ShareWindow;
}

class ShareWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ShareWindow(QWidget *parent = nullptr);
    ~ShareWindow();
    void init_window();

public:
    void show_friend_list(QString shareFileName);
    void handle_online_friends_reponse(const QStringList& content);

private slots:
    void on_bt_allcheck_clicked();

    void on_all_notcheck_clicked();

    void on_bt_share_clicked();

    void on_bt_cancel_clicked();

private:
    Ui::ShareWindow *ui;
    QString m_shareFileName;
};

#endif // SHAREWINDOW_H
