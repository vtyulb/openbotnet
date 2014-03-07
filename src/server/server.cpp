#include "server.h"

Server::Server(QObject *parent): QTcpServer(parent) {
    loadRSAkey();
    bool goodStart = listen(QHostAddress::Any, 24953);
    if (!goodStart) {
        printf("Can't reserve port #24953\n");
        exit(0);
    } else {
        printf("listening on 24953\n");
        bots = new QSet<Bot*>;
    }
}

void Server::loadRSAkey() {
    QFile file(QString("private"));
    file.open(QIODevice::ReadOnly);
    QByteArray key = file.readAll();
    ByteQueue q;

    for (int i = 0; i < key.size(); i++)
        q.Put(key[i]);

    privateKey.Load(q);
}

void Server::incomingConnection(qintptr handle) {
    Bot *bot = new Bot(handle, &privateKey, this);

    emit log(bot->peerAddress().toString() + " connected");

    usingBots.lock();
    bots->insert(bot);
    usingBots.unlock();

    QObject::connect(bot, SIGNAL(deleteMe(Bot*)), this, SLOT(deleteBot(Bot*)));
}

QVector<Bot*> Server::getBots() {
    QVector<Bot*> result;
    usingBots.lock();
    for (QSet<Bot*>::Iterator i = bots->begin(); i != bots->end(); i++)
        result.push_back(*i);
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
    emit log("removing bot" + bot->peerAddress().toString());
    usingBots.unlock();
}

void Server::sendMessage(QByteArray data) {
    emit log("Broadcasting " + data);
    for (QSet<Bot*>::Iterator i = bots->begin(); i != bots->end(); i++)
        (*i)->safeWrite(data);
}
