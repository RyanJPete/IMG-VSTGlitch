#include "imagevsthostg.h"
#include "ImageVSTHost.h"

#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageVSTHostG w;
    w.show();

    return a.exec();
}
