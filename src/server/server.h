#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSet>
#include <QMutex>
#include <QVector>

#include <bot.h>

class Server : public QTcpServer {
    Q_OBJECT
    public:
        explicit Server(QObject *parent = 0);

        QVector<QHostAddress> getBots();
        Bot *getBot(QHostAddress);

    protected:
        void incomingConnection(qintptr handle);

    private:
        QSet<Bot*> *bots;
        QMutex usingBots;

    signals:
        void log(QString);
        void stop();

    private slots:
        void deleteBot(Bot*);

    public slots:
        void sendMessage(QByteArray);
};

#endif // SERVER_H
