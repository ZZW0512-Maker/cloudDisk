#include "cloudclient.h"
#include "basewindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CloudClient::get_instance().show();

    return a.exec();
}
