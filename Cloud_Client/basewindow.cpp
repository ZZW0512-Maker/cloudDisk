#include "basewindow.h"
#include "ui_basewindow.h"
#include "filewindow.h"
#include "friendswindow.h"

BaseWindow::BaseWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BaseWindow)
  , friendswd(new FriendsWindow)
  , filewd(new FileWindow)
{
    ui->setupUi(this);

    while(ui->stackedWidget->count() != 0)
    {
        ui->stackedWidget->removeWidget(ui->stackedWidget->currentWidget());
    }
    ui->stackedWidget->addWidget(friendswd);
    ui->stackedWidget->addWidget(filewd);

    //ui->stackedWidget->setCurrentWidget(friendswd);
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));

    this->get_file_window()->fileWindow_init();
}

BaseWindow &BaseWindow::get_instance()
{
    static BaseWindow instance;
    return instance;
}

BaseWindow::~BaseWindow()
{
    delete ui;
}

FriendsWindow *BaseWindow::get_friend_window()
{
    return friendswd;
}

FileWindow *BaseWindow::get_file_window()
{
    return filewd;
}
