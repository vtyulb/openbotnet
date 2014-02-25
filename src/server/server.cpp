#include "server.h"

Server::Server(QObject *parent): QObject(parent) {
    server = new QTcpServer(this);
    QObject::connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    server->listen(QHostAddress::Any, 24953);
    bots = new QSet<QTcpSocket*>;
}

void Server::newConnection() {
    while (server->hasPendingConnections())
        openConnection(server->nextPendingConnection());
}

void Server::openConnection(QTcpSocket *socket) {
    qDebug() << "connection established" << socket->peerAddress();

    usingBots.lock();
    bots->insert(socket);
    usingBots.unlock();

    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(somebodyHasSomethingToSay()));
    QObject::connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
}

void Server::somebodyHasSomethingToSay() {
    deleteDeadBots();
    for (QSet<QTcpSocket*>::Iterator i = bots->begin(); i != bots->end(); i++) {
        QTcpSocket *socket = *i;
        while (socket->canReadLine())
            qDebug() << socket->readLine() << "from" << socket->peerAddress();
    }
}

QVector<QHostAddress> Server::getBots() {
    deleteDeadBots();
    QVector<QHostAddress> result;

    usingBots.lock();
    for (QSet<QTcpSocket*>::Iterator i = bots->begin(); i != bots->end(); i++)
        result.push_back((*i)->peerAddress());
    usingBots.unlock();

    return result;
}

void Server::deleteDeadBots() {
    usingBots.lock();
    for (QSet<QTcpSocket*>::Iterator i = bots->begin(); i != bots->end(); i++)
        if (!(*i)->isValid()) {
            bots->remove(*i);
            usingBots.unlock();
            deleteDeadBots();
            return;
        }
    usingBots.unlock();
}
