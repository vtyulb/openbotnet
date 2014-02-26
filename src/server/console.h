#ifndef CONSOLE_H
#define CONSOLE_H

#include <QThread>
#include <QTextStream>
#include <QCoreApplication>
#include <QMutex>
#include <QTimer>
#include <QFile>

#include <server.h>
#include <bot.h>

class Console : public QThread
{
    Q_OBJECT
    public:
        explicit Console(Server *server, QObject *parent = 0);

    protected:
        void run();

    private:
        Server *server;
        QMutex printing;
        QTextStream *in;
        Bot *bot;

        void invite();

    signals:
        void tellBot(QByteArray);

    private slots:
        void processData();
        void log(QString);
        void dataFromBot(Bot*, QByteArray);

};

#endif // CONSOLE_H
