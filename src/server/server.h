#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSet>
#include <QMutex>
#include <QVector>

class Server : public QObject {
    Q_OBJECT
    public:
        explicit Server(QObject *parent = 0);

        QVector<QHostAddress> getBots();

    private:
        QTcpServer *server;
        QSet<QTcpSocket*> *bots;
        QMutex usingBots;

        void openConnection(QTcpSocket*);
        void deleteDeadBots();

    signals:

    private slots:
        void newConnection();
        void somebodyHasSomethingToSay();
};

#endif // SERVER_H
