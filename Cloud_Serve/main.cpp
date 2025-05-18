#include "cloudserve.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CloudServe::get_instance().show();

    return a.exec();
}
