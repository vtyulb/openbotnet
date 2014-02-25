#include "console.h"

Console::Console(Server *server, QObject *parent): QThread(parent), server(server) {
}

void Console::run() {
    QTextStream in(stdin);
    while (1) {
        printf(" > ");
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
    }
}
