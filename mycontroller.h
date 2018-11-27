#ifndef MYCONTROLLER_H
#define MYCONTROLLER_H

#include "invoice.h"
#include <QString>
#include <QThread>


class QThread_Mid: public QThread{
    virtual void run_1() = 0;
    void run(){
        run_1();
    }
};

class MyController: public QThread_Mid, public Invoice
{
    Q_OBJECT
public:
    MyController();
    void message(int, int);
    void run_1();
private:

signals:
    void newMessage(QString, QString);
};

#endif // MYCONTROLLER_H