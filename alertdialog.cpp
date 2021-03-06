#include "alertdialog.h"
#include "ui_alertdialog.h"
#include <QCloseEvent>
#include <QTimer>

AlertDialog::AlertDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlertDialog)
{
    //setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateText()));
    time_left = 1;
    timer->start(1000);
    isClosed = false;
}

AlertDialog::~AlertDialog()
{
    delete ui;
    delete timer;
}

void AlertDialog::setText_0(QString msg){
    ui->label_0->setText(msg);
}
void AlertDialog::setText_1(QString msg){

}

void AlertDialog::closeEvent(QCloseEvent *event){
    if(!isClosed){
        emit closed();
        isClosed = true;
    }
    timer->stop();
    QDialog::closeEvent(event);
}

void AlertDialog::updateText(){
    --time_left;
    QString str = "(";
    str.append(QString::number(time_left));
    str.append(")");
    ui->label->setText(str);
    if(time_left <=0 ){
        timer->stop();
        this->close();
    }
}

