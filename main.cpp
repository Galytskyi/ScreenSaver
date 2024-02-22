#include "MainWindow.h"
#include "Options.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("ScreenSaver");
    a.setOrganizationName("ScreenSaver");

    theOptions.load();

    MainWindow w;
    w.show();

    return a.exec();
}
