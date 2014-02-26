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
    if (!canReadLine())
        return;

    QByteArray array = readLine();
    if (array[0] == 'D') {
        currentDirectory = array.right(array.size() - 1).left(array.size() - 2);
        emit dataAvailable(this, QByteArray());
        return;
    }

    while (canReadLine()) {
        QByteArray a = readLine();
        if (a[0] == 'D')
            continue;

        array += a.right(a.size() - 1);
    }
    emit dataAvailable(this, array);
}

void Bot::safeWrite(QByteArray data) {
    writing.lock();
    write(data);
    writing.unlock();
}

QByteArray Bot::getCWD() {
    return currentDirectory.toUtf8();
}
