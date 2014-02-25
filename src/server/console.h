#ifndef CONSOLE_H
#define CONSOLE_H

#include <QThread>
#include <QTextStream>
#include <QCoreApplication>

#include <server.h>

class Console : public QThread
{
    Q_OBJECT
    public:
        explicit Console(Server *server, QObject *parent = 0);

    protected:
        void run();
        Server *server;

    signals:

    public slots:
};

#endif // CONSOLE_H
