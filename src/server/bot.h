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

using namespace CryptoPP;

class Bot : public QTcpSocket
{
    Q_OBJECT
    public:
        explicit Bot(qintptr handle, QObject *parent = 0);
        ~Bot();

        bool operator < (const Bot &bot) { return this->socketDescriptor() < bot.socketDescriptor(); }

        QByteArray getCWD();

    private:
        QMutex writing;
        QString currentDirectory;

        CFB_Mode<AES>::Encryption *cfbEncryption;
        CFB_Mode<AES>::Decryption *cfbDecryption;

        QByteArray decrypt(QByteArray);
        QByteArray encrypt(QByteArray);



    signals:
        void deleteMe(Bot*);
        void dataAvailable(Bot *, QByteArray);

    public slots:
        void readSomething();
        void safeWrite(QByteArray);

    private slots:
        void initAES();
};

#endif // BOT_H
