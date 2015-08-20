#ifndef SIGNDIALOG_H
#define SIGNDIALOG_H

#include <QDialog>

namespace Ui {
    class SignDialog;
}

class SignDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignDialog(QWidget *parent = 0);
    ~SignDialog();

    QString  m_sig;

private:
    Ui::SignDialog *ui;



private slots:

    void on_btnSaveSignature_clicked();
    void on_btnClearSignature_clicked();
    void on_btnCancel_clicked();


};

#endif // SIGNDIALOG_H
