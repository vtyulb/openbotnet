#ifndef BOT_H
#define BOT_H

#include <QTcpSocket>
#include <QHostAddress>
#include <QMutex>

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

    signals:
        void deleteMe(Bot*);
        void dataAvailable(Bot *, QByteArray);

    public slots:
        void readSomething();
        void safeWrite(QByteArray);
};

#endif // BOT_H
