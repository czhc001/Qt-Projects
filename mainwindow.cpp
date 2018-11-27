﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "managementwindow.h"
#include "yesruledialog.h"
#include <QFileDialog>

MainWindow::MainWindow(int userid, int permissionid, QString username, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->userid = userid;
    this->username = username;
    this->permissionid = permissionid;
    ui->setupUi(this);

    if(permissionid != 1)
        ui->Button_manage->setEnabled(false);
    QString m_sProjectPath = tr("D:\\images");
    QDir dir(m_sProjectPath);
    dir.setFilter(QDir::Files);
    QFileInfoList fileList = dir.entryInfoList();
    int fileCount = fileList.count();
    for(int i=0;i<fileCount;++i){
        QString filepath = fileList[i].absoluteFilePath();
        qDebug() << filepath;
        QImage *img = new QImage(filepath);
        image_list.append(img);
    }


    timer = new QTimer(this);
    ii = 0;
    connect(timer, SIGNAL(timeout()), this, SLOT(updateImage()));
    timer->start(50);
    control = new MyController();
    connect(control, SIGNAL(newMessage(QString, QString)), this, SLOT(on_New_Message(QString, QString)));
    isRunning = false;
    started = false;
    firstStarted = true;
    checking_current = false;
    initializa_UI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializa_UI(){
    QPalette palette;
    palette.setColor(QPalette::Background, Qt::darkGray);
    this->setPalette(palette);

    palette.setColor(QPalette::Background, Qt::white);
    ui->lineEdit_noresult->setAutoFillBackground(true);
    ui->lineEdit_noresult->setPalette(palette);

    palette.setColor(QPalette::Background, Qt::white);
    ui->lineEdit_coderesult->setAutoFillBackground(true);
    ui->lineEdit_coderesult->setPalette(palette);
}

void MainWindow::on_Button_manage_clicked()
{
    ManagementWindow * managementWindow = new ManagementWindow(userid, permissionid);
    managementWindow->show();
}

void MainWindow::updateImage(){
    //qDebug() << ii;

    //ui->label_image->setPixmap(QPixmap::fromImage(*image_list.at(ii)));
    //ii = (ii+1)%image_list.size();
}

void MainWindow::on_Button_start_clicked()
{
    QMutexLocker locker(&start_mutex);
    if(!started){
        started = true;
        QString dep = ui->lineEdit_department->text();
        QString ser = ui->lineEdit_serialnumber->text();
        QString manager_name = ui->lineEdit_manage->text();
        QNetworkRequest request;
        QString url_str;
        QDateTime current_time = ui->dateEdit->dateTime();
        QString StrCurrentTime = current_time.toString("yyyyMMdd");
        url_str.append("http://120.78.190.36:9601/begin?userid=");
        url_str.append(QString::number(userid));
        url_str.append("&date=");
        url_str.append(StrCurrentTime);
        url_str.append("&department=");
        url_str.append(dep);
        url_str.append("&serialnumber=");
        url_str.append(ser);
        url_str.append("&manager=");
        url_str.append(manager_name);
        qDebug() << url_str;
        request.setUrl(QUrl(url_str));
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkReply *reply = manager->get(request);
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(on_Begin_Reply(QNetworkReply*)));
        ui->lineEdit_department->setEnabled(false);
        ui->lineEdit_serialnumber->setEnabled(false);
        ui->lineEdit_manage->setEnabled(false);
        ui->dateEdit->setEnabled(false);
        ui->Button_start->setText(QString::fromLocal8Bit("结  束"));
    }
    else{
        ui->lineEdit_department->setEnabled(true);
        ui->lineEdit_serialnumber->setEnabled(true);
        ui->lineEdit_manage->setEnabled(true);
        ui->dateEdit->setEnabled(true);
        ui->Button_start->setText(QString::fromLocal8Bit("开  始"));
        isRunning = false;
        started = false;
    }
}

void MainWindow::on_New_Message(QString message0, QString message1){
    if(isRunning && started && ! checking_current){
        new_message = true;
        current_code = message0;
        current_no = message1;
        ui->lineEdit_coderesult->setText(current_code);
        ui->lineEdit_noresult->setText(current_no);
    }
}

void MainWindow::on_Query_Result(QNetworkReply* reply){
    QByteArray data = reply->readAll();
    QString result = QString::fromStdString(data.toStdString()).toUtf8();
    qDebug() << result;
    QJsonDocument document;
    QJsonParseError ParseError;
    document =QJsonDocument::fromJson(result.toUtf8(),&ParseError);
    if(!document.isNull() && ParseError.error == QJsonParseError::NoError){
        if(document.isObject()){
            QJsonObject Object = document.object();
            if(!Object.isEmpty()){
                if(Object.contains("data")){
                    QJsonValue dataObj = Object.value("data");
                    if(dataObj.isBool()){
                        bool result = dataObj.toBool();
                        if(result){
                            on_YesRule(userid, versionid, current_code, current_no);
                        }
                        else{

                        }
                    }
                    else if(dataObj.isString()){
                        QString dataStr = dataObj.toString();
                        if(QString::compare("norule", dataStr, Qt::CaseSensitive) == 0){
                            isRunning = false;
                            ui->Button_start->setEnabled(false);
                            YesruleDialog *dialog = new YesruleDialog(userid, versionid, current_code, current_no);
                            connect(dialog, SIGNAL(NoRule(int, int, QString, QString)), this, SLOT(on_NoRule(int, int, QString, QString)));
                            connect(dialog, SIGNAL(YesRule(int, int, QString, QString)), this, SLOT(on_YesRule(int, int, QString, QString)));
                            dialog->show();
                        }
                    }
                    else{

                    }
                }
            }
        }
    }
}

void MainWindow::on_YesRule(int userid, int versionid, QString code, QString no){
    ui->Button_start->setEnabled(true);
    ui->Button_check->setText(QString::fromLocal8Bit("检  测"));
    ui->Button_check->setEnabled(true);
    ui->lineEdit_coderesult->setEnabled(true);
    ui->lineEdit_noresult->setEnabled(true);
    checking_current = false;
}

void MainWindow::on_NoRule(int userid, int versionid, QString code, QString no){
    ui->Button_start->setEnabled(true);
    ui->Button_start->setText(QString::fromLocal8Bit("开  始"));
    isRunning = false;
    started = false;
    checking_current = false;
}

void MainWindow::on_Begin_Reply(QNetworkReply* reply){
    QMutexLocker locker(&start_mutex);
    QByteArray data = reply->readAll();
    QString result = QString::fromStdString(data.toStdString()).toUtf8();
    qDebug() << result;
    QJsonDocument document;
    QJsonParseError ParseError;
    document =QJsonDocument::fromJson(result.toUtf8(),&ParseError);
    if(!document.isNull() && ParseError.error == QJsonParseError::NoError){
        if(document.isObject()){
            QJsonObject Object = document.object();
            if(!Object.isEmpty()){
                if(Object.contains("data")){
                    QJsonValue dataObj = Object.value("data");
                    if(dataObj.isObject()){  //判断是否是对象
                        QJsonObject object = dataObj.toObject();
                        versionid = object.value("versionid").toInt();

                        isRunning = true;
                        if(firstStarted){
                            control->start();
                            firstStarted = false;
                        }
                    }
                    else if(dataObj.isBool()){

                    }
                }
            }
        }
    }
}


void MainWindow::on_Button_check_clicked(){
    QMutexLocker locker(&check_mutex);
    if(isRunning && !checking_current && new_message){
        checking_current = true;
        new_message = false;
        ui->Button_check->setEnabled(false);
        ui->Button_check->setText(QString::fromLocal8Bit("检测中"));
        ui->lineEdit_coderesult->setEnabled(false);
        ui->lineEdit_noresult->setEnabled(false);
        QNetworkRequest request;
        QString url_str;
        url_str.append("http://120.78.190.36:9601/queryrule?userid=");
        url_str.append(QString::number(userid));
        url_str.append("&versionid=");
        url_str.append(QString::number(versionid));
        url_str.append("&invoicecode=");
        url_str.append(current_code);
        url_str.append("&invoicenumber=");
        url_str.append(current_no);
        request.setUrl(QUrl(url_str));
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkReply *reply = manager->get(request);
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(on_Query_Result(QNetworkReply*)));
    }
}

void MainWindow::on_lineEdit_coderesult_editingFinished()
{
    if(isRunning && started && ! checking_current){
        new_message = true;
        current_code = ui->lineEdit_coderesult->text();
    }
}

void MainWindow::on_lineEdit_noresult_editingFinished()
{
    if(isRunning && started && ! checking_current){
        new_message = true;
        current_no = ui->lineEdit_noresult->text();
    }
}
