#include "tcpserver.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyleSheet(
        "* { font-family: \"Microsoft YaHei\", \"Segoe UI\", sans-serif; }"
    );
    TcpServer w;
    w.show();
    return a.exec();
}
