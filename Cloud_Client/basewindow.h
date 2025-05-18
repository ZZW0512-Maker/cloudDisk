#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QWidget>
#include "friendswindow.h"
#include "filewindow.h"

namespace Ui {
class BaseWindow;
}

class BaseWindow : public QWidget
{
    Q_OBJECT

private:
    explicit BaseWindow(QWidget *parent = nullptr);
public:
    static BaseWindow& get_instance();
    ~BaseWindow();

public:
    FriendsWindow* get_friend_window();
    FileWindow* get_file_window();

private:
    Ui::BaseWindow *ui;
    FriendsWindow *friendswd;
    FileWindow * filewd;
};

#endif // BASEWINDOW_H
