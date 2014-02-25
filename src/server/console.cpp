#include "console.h"

Console::Console(Server *server, QObject *parent): QThread(parent), server(server) {
}

void Console::run() {
    printf(" > ");
    in = new QTextStream(stdin);
    QTimer *timer = new QTimer;
    timer->setInterval(100);
    timer->start();
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(processData()), Qt::DirectConnection);
    QObject::connect(server, SIGNAL(log(QString)), this, SLOT(log(QString)));
    exec();
}

void Console::processData() {
    printing.lock();
    if (in->atEnd())
        return;

    QString original = in->readLine();
    QString s = original.toUpper();
    if (s == "EXIT")
        QCoreApplication::instance()->quit();
    else if (s == "BOTLIST") {
        printf("bots: \n");
        QVector<QHostAddress> bots = server->getBots();
        for (int i = 0; i < bots.size(); i++)
            printf("%d: %s\n", i, bots[i].toString().toUtf8().constData());
    } else if (s == "HELP") {
        printf("help\nbotlist\nexit\n");
    } else {
        server->sendMessage(original.toUtf8());
    }

    printf(" > ");
    printing.unlock();
}

void Console::log(QString message) {
    printing.lock();
    printf(" %s\n", message.toUtf8().constData());
    printf(" > ");
    printing.unlock();
}
