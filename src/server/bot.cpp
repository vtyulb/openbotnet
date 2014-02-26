#include "bot.h"

Bot::Bot(qintptr handle, QObject *parent): QTcpSocket(parent) {
    setSocketDescriptor(handle);
    QObject::connect(this, SIGNAL(readyRead()), this, SLOT(readSomething()));
    QObject::connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}

Bot::~Bot() {
    emit deleteMe(this);
}

void Bot::readSomething() {
    if (!canReadLine())
        return;

    QByteArray array;
    while (canReadLine()) {
        QByteArray a = readLine();
        if (a[0] == 'D')
            currentDirectory = a.right(a.size() - 1).left(a.size() - 2);
        else {
            array += a.right(a.size() - 1);
        }

        if (!canReadLine())
            waitForReadyRead(500);
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
