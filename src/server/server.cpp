#include "server.h"

Server::Server(QObject *parent): QTcpServer(parent) {
    bool goodStart = listen(QHostAddress::Any, 24953);
    if (!goodStart) {
        printf("Can't reserve port #24953\n");
        exit(0);
    } else {
        printf("listening on 24953\n");
        bots = new QSet<Bot*>;
    }
}

void Server::incomingConnection(qintptr handle) {
    Bot *bot = new Bot(handle, this);

    emit log(bot->peerAddress().toString() + " connected");

    usingBots.lock();
    bots->insert(bot);
    usingBots.unlock();

    QObject::connect(bot, SIGNAL(deleteMe(Bot*)), this, SLOT(deleteBot(Bot*)));
    QObject::connect(bot, SIGNAL(readyRead()), this, SLOT(somebodyHasSomethingToSay()));
}

void Server::somebodyHasSomethingToSay() {
    for (QSet<Bot*>::Iterator i = bots->begin(); i != bots->end(); i++) {
        QTcpSocket *socket = *i;
        while (socket->canReadLine())
            qDebug() << socket->readLine() << "from" << socket->peerAddress();
    }
}

QVector<QHostAddress> Server::getBots() {
    QVector<QHostAddress> result;

    usingBots.lock();
    for (QSet<Bot*>::Iterator i = bots->begin(); i != bots->end(); i++)
        result.push_back((*i)
                         ->peerAddress());
    usingBots.unlock();

    return result;
}

Bot *Server::getBot(QHostAddress address) {
    for (QSet<Bot*>::Iterator i = bots->begin(); i != bots->end(); i++)
        if ((*i)->peerAddress() == address)
            return *i;

    return NULL;
}

void Server::deleteBot(Bot *bot) {
    usingBots.lock();
    bots->remove(bot);
    usingBots.unlock();
}

void Server::sendMessage(QByteArray data) {
    emit log("Broadcasting " + data);
    for (QSet<Bot*>::Iterator i = bots->begin(); i != bots->end(); i++)
        (*i)->safeWrite(data);
}
