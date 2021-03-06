#ifndef YESRULEDIALOG_H
#define YESRULEDIALOG_H

#include <QDialog>
#include <QtNetwork>
#include <QTimer>
#include <QStandardItemModel>
#include <QJsonObject>
#include <QList>

namespace Ui {
class yesruleDialog;
}

class YesruleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit YesruleDialog(int userid, int versionid, int current_count, QString current_code, QString current_no, QString current_manager,
                           QString current_department, QString current_date, QString current_serial, QString current_username,
                           QJsonArray hisInfoArray, QWidget *parent = nullptr);
    ~YesruleDialog();

private slots:
    void on_Button_yes_clicked();

    void on_Button_no_clicked();

    void on_textEdit_textChanged();

    void on_Yes(QNetworkReply*);

    void on_No(QNetworkReply*);

    void on_TimerClose();

private:
    Ui::yesruleDialog *ui;
    int userid;
    int versionid;
    QString current_code;
    QString current_no;
    int current_count;
    QTimer *timer;
    void addRow(QString, QString, QList<QJsonObject>, int, QString);
    QStandardItemModel *dataModel;

signals:
    void NoRule(int userid, int versionid, QString code, QString no);
    void YesRule(int userid, int versionid, QString code, QString no);
    void Complete(int userid, int versionid, QString code, QString no);
};

#endif // YESRULEDIALOG_H
