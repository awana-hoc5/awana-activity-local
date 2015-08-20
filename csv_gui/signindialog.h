#ifndef SIGNINDIALOG_H
#define SIGNINDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QTimer>
#include <QListWidgetItem>
#include <QImage>

namespace Ui {
    class SigninDialog;
}

class SigninDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SigninDialog(QWidget *parent = 0);
    ~SigninDialog();

    void setScanMode(int scan);



protected:

    void keyPressEvent(QKeyEvent* event);

private:
    void findSignin();
    bool searchSignin(int group);
    void clearSignin();
    void updateUI();
    void checkSignin(QString t);

    Ui::SigninDialog *ui;
    // QTimer * m_timer;

    QString m_barCode;
    QString m_grade;
    QString m_gender;
    QString m_color;

    int m_group;
    int m_signinState;
    QSqlDatabase m_database;

    int m_curLen;
    QString m_enter;

    QImage m_imageRed;
    QImage m_imageGreen;

signals:

    void receivedBarCode();

    void sigGotSignin(QString name, QString color, int group, bool isSunday, bool isUniform, bool isLate, bool isBadge);


private slots:
   // void onTimeout();
    void onReceiveBarCode();
    void cbBadgeChange(int s);
    void cmbTextChanged(const QString);
    void listItemSelected(QListWidgetItem * item);
    //void listItemDecided(QListWidgetItem * item);

    void on_btnOKSignin_clicked();
    void on_btnCancelSignin_clicked();
    void on_btnContinue_clicked();


};

#endif // SIGNINDIALOG_H
