#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSet>
#include <QMutex>
#include <QVector>
#include <QFile>

#include <bot.h>
#include <cryptopp/rsa.h>
#include <cryptopp/files.h>
#include <cryptopp/base64.h>
#include <../bot/base64.h>

class Server : public QTcpServer {
    Q_OBJECT
    public:
        explicit Server(QObject *parent = 0);

        QVector<Bot*> getBots();
        Bot *getBot(QHostAddress);
        void dumpWhitelist();
        RSA::PrivateKey getPrivateKey();

    protected:
        void incomingConnection(qintptr handle);

    private:
        QSet<Bot*> *bots;
        QMutex usingBots;

        RSA::PrivateKey privateKey;
        void loadRSAkey();

    signals:
        void log(QString);
        void stop();

    private slots:
        void deleteBot(Bot*);

    public slots:
        void sendMessage(QByteArray);
};

#endif // SERVER_H
