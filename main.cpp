#include "chyprlandwelcome.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CHyprlandWelcome w;
    w.show();
    return a.exec();
}
