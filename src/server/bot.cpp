#include "bot.h"

Bot::Bot(qintptr handle, QObject *parent): QTcpSocket(parent) {
    setSocketDescriptor(handle);
    QObject::connect(this, SIGNAL(readyRead()), this, SLOT(readSomething()));
    QObject::connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    qDebug() << "connection established" << peerAddress();
}

Bot::~Bot() {
    emit deleteMe(this);
}

void Bot::readSomething() {
    while (canReadLine())
        qDebug() << this->peerAddress() << ":" << this->readLine();
}



