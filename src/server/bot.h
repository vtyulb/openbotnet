#ifndef BOT_H
#define BOT_H

#include <QTcpSocket>
#include <QHostAddress>

class Bot : public QTcpSocket
{
    Q_OBJECT
    public:
        explicit Bot(qintptr handle, QObject *parent = 0);
        ~Bot();

        bool operator < (const Bot &bot) { return this->socketDescriptor() < bot.socketDescriptor(); }

    signals:
        void deleteMe(Bot*);

    public slots:
        void readSomething();

};

#endif // BOT_H
