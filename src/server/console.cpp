#include "console.h"

Console::Console(Server *server, QObject *parent): QThread(parent), server(server) {
}

void Console::run() {
    QTextStream in(stdin);
    printf(" > ");
    while (1) {
        if (in.atEnd()) {
            msleep(10);
            continue;
        }

        QString s = in.readLine().toUpper();
        if (s == "EXIT") {
            QCoreApplication::instance()->quit();
            break;
        }

        if (s == "BOTLIST") {
            printf("bots: \n");
            QVector<QHostAddress> bots = server->getBots();
            for (int i = 0; i < bots.size(); i++)
                printf("%d: %s\n", i, bots[i].toString().toUtf8().constData());
        }

        if (s == "HELP") {
            printf("help\nbotlist\nexit\n");
        }


        printf(" > ");
    }
}
