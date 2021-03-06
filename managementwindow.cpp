﻿#include "managementwindow.h"
#include "ui_managementwindow.h"
#include "modifyuserdialog.h"
#include "rrdialog.h"
#include <QDebug>
#include <QList>
#include <severaddress.h>

ManagementWindow::ManagementWindow(int userid, int permissionid, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ManagementWindow)
{
    //setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());
    this->userid = userid;
    this->permissionid = permissionid;
    ui->setupUi(this);

    dataModel = new QStandardItemModel();
    dataModel->setColumnCount(4);
    dataModel->setHeaderData(USERNAME_COLUMN, Qt::Horizontal, QString::fromLocal8Bit("用户名"));
    dataModel->setHeaderData(PERMISSIONID_COLUMN, Qt::Horizontal, QString::fromLocal8Bit("权限"));
    dataModel->setHeaderData(PASSWORDS_COLUMN, Qt::Horizontal, QString::fromLocal8Bit("密码"));
    dataModel->setHeaderData(ID_COLUMN, Qt::Horizontal, QString::fromLocal8Bit("ID"));
    ui->tableView_userInfo->setModel(dataModel);
    ui->tableView_userInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_userInfo->setColumnWidth(0, 190);
    ui->tableView_userInfo->setColumnWidth(1, 190);
    ui->tableView_userInfo->setColumnWidth(2, 190);
    ui->tableView_userInfo->setColumnWidth(3, 190);
    hid();
    connect(ui->tableView_userInfo->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
            this,
            SLOT(onCurrentRowChanged(const QModelIndex &, const QModelIndex &)));
    queryAllUser();
}

ManagementWindow::~ManagementWindow()
{
    delete ui;
}

void ManagementWindow::on_Button_addUser_clicked()
{
    addUserDialog = new AddUserDialog(userid);
    connect(addUserDialog, SIGNAL(user_Added(QString)), this, SLOT(addedNewUser(QString)));
    addUserDialog->show();
}

void ManagementWindow::addedNewUser(QString username){
    //qDebug() << "SLOT GOT  " <<username << ": " <<passwords;
    //QList<QStandardItem *> new_row;
    //new_row.append(new QStandardItem(username));
    //new_row.append(new QStandardItem(passwords));
    //dataModel->appendRow(new_row);
    queryAllUser();
}

void ManagementWindow::deletedUser(QNetworkReply* reply){
    QMutexLocker locker(&mutex);
    queryAllUser();
    delete reply;
}

void ManagementWindow::obtainAllUser(QNetworkReply* reply){
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
                    qDebug() << dataObj;
                    if(dataObj.isArray()){  //判断是否是对象
                        if(dataModel->rowCount() > 0)
                            dataModel->removeRows(0, dataModel->rowCount());
                        QJsonArray users_info = dataObj.toArray();
                        for(int i = 0; i < users_info.size(); ++i){
                            QJsonObject user_info = users_info.at(i).toObject();
                            QString current_username = user_info.value("username").toString();
                            QString current_password = user_info.value("password").toString();
                            int current_userid = user_info.value("userid").toInt();
                            int current_permissionid = user_info.value("permissionid").toInt();
                            qDebug() << current_username << ": " << current_password;
                            QList<QStandardItem *> list;
                            QStandardItem * item_username = new QStandardItem(current_username);
                            QStandardItem * item_permissionid = new QStandardItem();
                            QStandardItem * item_userid = new QStandardItem();
                            QStandardItem * item_passwords = new QStandardItem(current_password);

                            item_username->setTextAlignment(Qt::AlignCenter);
                            item_permissionid->setTextAlignment(Qt::AlignCenter);
                            item_userid->setTextAlignment(Qt::AlignCenter);
                            item_passwords->setTextAlignment(Qt::AlignCenter);

                            item_permissionid->setData(QVariant(current_permissionid),Qt::EditRole);
                            item_userid->setData(QVariant(current_userid),Qt::EditRole);

                            list.append(item_username);
                            list.append(item_permissionid);
                            list.append(item_passwords);
                            list.append(item_userid);

                            dataModel->appendRow(list);
                        }
                        hid();
                    }
                    else{
                        qDebug() << "No Array";
                    }
                }
                else{
                    qDebug() << "No data";
                }
            }
        }
        else{
            qDebug() << "No Object";
        }
    }
    else{
        qDebug() << "Error!";
    }
    delete reply;
}

void ManagementWindow::queryAllUser(){
    QString url_str;
    QNetworkRequest request;
    url_str.append("http://" + SeverAddress::address + "/alluser?userid=");
    url_str.append(QString::number(userid));
    url_str.append("&permissionid=");
    url_str.append(QString::number(permissionid));
    request.setUrl(QUrl(url_str));
    //qDebug()<<request.url();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);
    //连接请求结束信号
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(obtainAllUser(QNetworkReply*)));
}

void ManagementWindow::on_Yes_Remove(){
    QItemSelectionModel *smodel = ui->tableView_userInfo->selectionModel();
    QModelIndex current_index = smodel->currentIndex();
    QString selected_username = dataModel->index(current_index.row(), USERNAME_COLUMN).data().toString();
    int selected_id = dataModel->index(current_index.row(), ID_COLUMN).data().toInt();
    tryDeleteUser(selected_id);
    /*
    QItemSelectionModel *smodel = ui->tableView_userInfo->selectionModel();
    QModelIndexList indexs = smodel->selectedRows();
    qDebug() << "";

    QList<int > selected_index;
    for(int i = 0; i < indexs.size(); ++i){
        selected_index.append(indexs.at(i).row());
    }
    qSort(selected_index.begin(), selected_index.end());

    QList<int> selected_uids;
    for(int i = 0; i < selected_index.size(); ++i){
        int current_uid = dataModel->index(selected_index.at(i), ID_COLUMN).data().toInt();
        selected_uids.append(current_uid);
        qDebug() << selected_index.at(i) << ": " << current_uid;
    }
    to_delete_item_count = selected_index.size();
    for(int i = 0; i < selected_uids.size(); ++i){
        tryDeleteUser(selected_uids.at(i));
    }
    */
    //delete rdialog;
}

void ManagementWindow::on_Button_deleteUser_clicked()
{
    QItemSelectionModel *smodel = ui->tableView_userInfo->selectionModel();
    QModelIndex current_index = smodel->currentIndex();
    qDebug() << current_index.row();
    if(current_index.row() < 0)
        return;
    QString selected_username = dataModel->index(current_index.row(), USERNAME_COLUMN).data().toString();
    int selected_id = dataModel->index(current_index.row(), ID_COLUMN).data().toInt();
    rdialog = new RRDialog(selected_username);
    connect(rdialog, SIGNAL(yes()), this, SLOT(on_Yes_Remove()));
    rdialog->show();
    /*
    QItemSelectionModel *smodel = ui->tableView_userInfo->selectionModel();
    QModelIndexList indexs = smodel->selectedRows();
    qDebug() << "";

    QList<int > selected_index;
    for(int i = 0; i < indexs.size(); ++i){
        selected_index.append(indexs.at(i).row());
    }
    qSort(selected_index.begin(), selected_index.end());

    QList<int> selected_uids;
    for(int i = 0; i < selected_index.size(); ++i){
        int current_uid = dataModel->index(selected_index.at(i), ID_COLUMN).data().toInt();
        selected_uids.append(current_uid);
        qDebug() << selected_index.at(i) << ": " << current_uid;
    }
    to_delete_item_count = selected_index.size();
    for(int i = 0; i < selected_uids.size(); ++i){
        tryDeleteUser(selected_uids.at(i));
    }
    */
}

void ManagementWindow::onCurrentRowChanged(const QModelIndex & current, const QModelIndex & previous){

}

void ManagementWindow::tryDeleteUser(int userid_todelete){
    QNetworkRequest request;
    QString url_str;
    url_str.append("http://" + SeverAddress::address + "/deleteuser?userid=");
    url_str.append(QString::number(this->userid));
    url_str.append("&bedeletedid=");
    url_str.append(QString::number(userid_todelete));
    request.setUrl(QUrl(url_str));
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);
    //连接请求结束信号
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(deletedUser(QNetworkReply*)));
}

void ManagementWindow::on_Button_modify_clicked()
{
    QItemSelectionModel *smodel = ui->tableView_userInfo->selectionModel();
    QModelIndex current_index = smodel->currentIndex();
    if(current_index.row() < 0)
        return;

    QString selected_username = dataModel->index(current_index.row(), USERNAME_COLUMN).data().toString();
    int selected_id = dataModel->index(current_index.row(), ID_COLUMN).data().toInt();
    ModifyUserDialog * modifyUserDialog = new ModifyUserDialog(userid, selected_id, selected_username);
    connect(modifyUserDialog, SIGNAL(modifiedUser(QString)), this, SLOT(modifiedUser(QString)));
    modifyUserDialog->show();
}

void ManagementWindow::modifiedUser(QString username){
    queryAllUser();
}

void ManagementWindow::hid(){
    ui->tableView_userInfo->setColumnHidden(PERMISSIONID_COLUMN,true);
    ui->tableView_userInfo->setColumnHidden(ID_COLUMN,true);
    //ui->tableView_userInfo->setColumnHidden(PASSWORDS_COLUMN,true);
}

void ManagementWindow::closeEvent(QCloseEvent *)
{
    //|窗口关闭之前需要的操作~
    emit windowClosed();
}

