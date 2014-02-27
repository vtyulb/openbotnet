#include <QCoreApplication>
#include <server.h>
#include <console.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server server;
    Console console(&server);
    console.start();

    QObject::connect(&console, SIGNAL(finished()), &a, SLOT(quit()));

    return a.exec();
}
