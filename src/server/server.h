#ifndef SERVER_H
#define SERVER_H

#include <QObject>
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

    protected:
        void incomingConnection(qintptr handle);

    private:
        QSet<Bot*> *bots;
        QMutex usingBots;

    signals:
        void log(QString);

    private slots:
        void somebodyHasSomethingToSay();
        void deleteBot(Bot*);
};

#endif // SERVER_H
