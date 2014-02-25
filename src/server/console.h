#ifndef CONSOLE_H
#define CONSOLE_H

#include <QThread>
#include <QTextStream>
#include <QCoreApplication>
#include <QMutex>
#include <QTimer>

#include <server.h>

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

    signals:

    private slots:
        void processData();
        void log(QString);

};

#endif // CONSOLE_H
