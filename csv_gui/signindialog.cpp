#include "signindialog.h"
#include "ui_signindialog.h"
#include "mainwindow.h"

#include <stdio.h>
#include <QKeyEvent>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTime>

SigninDialog::SigninDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SigninDialog)
{
    ui->setupUi(this);



    //connect(ui->cbBadge, SIGNAL(stateChanged(int)),  this, SLOT(cbBadgeChange(int)));
    connect(ui->leNameSignin, SIGNAL(textChanged (const QString)),  this, SLOT(cmbTextChanged(const QString)));

    connect(ui->lwNames, SIGNAL(itemClicked  ( QListWidgetItem *  )),  this, SLOT(listItemSelected(QListWidgetItem * )));
   // connect(ui->lwNames, SIGNAL(itemDoubleClicked ( QListWidgetItem *  )),  this, SLOT(listItemDecided(QListWidgetItem * )));

    connect(this, SIGNAL(receivedBarCode( )), this, SLOT(onReceiveBarCode( )), Qt::QueuedConnection);
    connect(this, SIGNAL(sigGotSignin(QString ,  QString ,int , bool , bool , bool, bool )), (MainWindow *)parent, SLOT(slotGotSignin(QString , QString , int , bool , bool , bool, bool )), Qt::QueuedConnection);


    m_imageRed.load("red.png");
    m_imageRed.load("green.png");

    m_signinState = 0;
    m_barCode = "";
    m_color = "red";
    m_enter = "";

    updateUI();
    m_database = QSqlDatabase::database();
    //m_timer = new QTimer(this);
    //connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));


    ui->lbFeedback->hide();
    m_curLen = 0;

}

void SigninDialog::setScanMode(int scan)
{
    if( scan){
        ui->cbBadge->setDisabled(true);
        ui->cbBadge->setChecked(true);
        //ui->leNameSignin->setDisabled(true);
        //showFullScreen();
    }
    else{
        ui->cbBadge->setDisabled(false);
        ui->cbBadge->setChecked(false);
        ui->leNameSignin->setDisabled(false);
        m_curLen = 0;
    }



}

SigninDialog::~SigninDialog()
{
    printf("~SigninDialog\n");
    //if(m_timer->isActive()) m_timer->stop();
    //delete m_timer;
    delete ui;
}

void SigninDialog::listItemSelected(QListWidgetItem * item)
{
    printf("listItemSelected \n");
    QString name = item->text();
    ui->leNameSignin->setText(name);
}

/*
void SigninDialog::listItemDecided(QListWidgetItem * item)
{
    printf("listItemDecided \n");

    QString name = item->text();
    ui->leNameSignin->setText(name);
    m_barCode = name;

    printf("get a scan:%s \n", qPrintable(m_barCode));

    emit receivedBarCode();
}*/

void SigninDialog::updateUI()
{


    switch(m_signinState){
    case 0:
        ui->btnContinue->setDisabled(true);
        ui->btnOKSignin->setDisabled(true);
        ui->btnCancelSignin->setDisabled(true);

        break;
    case 1:
        ui->btnContinue->setDisabled(true);
        ui->btnOKSignin->setDisabled(true);
        ui->btnCancelSignin->setDisabled(true);

        break;
    case 2:
        ui->btnContinue->setDisabled(true);
        ui->btnOKSignin->setDisabled(true);
        ui->btnCancelSignin->setDisabled(false);

        break;
    case 3:
        ui->btnContinue->setDisabled(false);
        ui->btnOKSignin->setDisabled(false);
        ui->btnCancelSignin->setDisabled(false);

        ui->leNameSignin->setDisabled(true);
        //ui->lwNames->setDisabled(true);

       // m_timer->stop();


        break;

    }
}

void SigninDialog::findSignin()
{

    ui->leGradeSignin->setText(m_grade);
    ui->leGenderSignin->setText(m_gender);
    if( m_color.contains("red",Qt::CaseInsensitive)){
        ui->lbColorSignin->setStyleSheet("QLabel { background-color : red;  }");
    }
    else if( m_color.contains("green",Qt::CaseInsensitive) ){
        ui->lbColorSignin->setStyleSheet("QLabel { background-color : green;  }");

    }
    else if( m_color.contains("blue",Qt::CaseInsensitive) ){
        ui->lbColorSignin->setStyleSheet("QLabel { background-color : blue;  }");

    }
    else if( m_color.contains("yellow",Qt::CaseInsensitive) ){
        ui->lbColorSignin->setStyleSheet("QLabel { background-color : yellow;  }");

    }
    else if(m_color.contains("brown", Qt::CaseInsensitive)){
        ui->lbColorSignin->setStyleSheet("QLabel { background-color : purple;  }");
        printf("unknown color:%s \n", qPrintable(m_color));

    }
    else{
        ui->lbColorSignin->setStyleSheet("QLabel { background-color : pink;  }");
        printf("unknown color:%s \n", qPrintable(m_color));

    }

    QTime curTime = QTime::currentTime();
    QTime lateTime = QTime::fromString("07:30 PM", "hh:mm AP");

    if(curTime < lateTime ){
        ui->cbLate->setChecked(false);
    }
    else{
        ui->cbLate->setChecked(true);
    }

    ui->cbLate->setDisabled(false);


    ui->btnContinue->setDisabled(false);

    //QString name, int group, bool isSunday, bool isUniform, bool isLate)
}



void SigninDialog::checkSignin(QString t)
{

    QString name = t.trimmed();

   // printf("name: %s\n", qPrintable(name) );
    if( name.length()== 0) return;

    ui->lwNames->clear();

    QStringList nameList;

    nameList.clear();

    QSqlQuery chkQuery(m_database);
    QString sqlStr;

    sqlStr = QString("select name from SininMembs where name like '%" + name + "%'");

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return ;
    }

    while( chkQuery.next() )
    {
        nameList << chkQuery.value(0).toString();

    }

    sqlStr = QString("select name from SparkSigninMembs where name like '%" + name + "%'");

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return ;
    }

    while ( chkQuery.next() )
    {
        nameList << chkQuery.value(0).toString();

    }
    sqlStr = QString("select name from CowkSigninMembs where name like '%" + name + "%'");

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return ;
    }

    while ( chkQuery.next() )
    {
        nameList << chkQuery.value(0).toString();

    }

    sqlStr = QString("select name from CubiSigninMembs where name like '%" + name + "%'");

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return ;
    }

    while ( chkQuery.next() )
    {
        nameList << chkQuery.value(0).toString();

    }


      ui->lwNames->addItems(nameList);

      if( nameList.size() == 1 && m_curLen < name.length()){
          ui->leNameSignin->setText(nameList[0]);

          ui->leNameSignin->setDisabled(true);

          m_curLen = nameList[0].length();

           ui->lbFeedback->hide();

      }
      else{

        m_curLen = name.length();
      }



}

void SigninDialog::cmbTextChanged(const QString text)
{
   // if( m_curLen < text.length())
    {

        //printf("cmbTextChanged:%s \n", qPrintable(text));
        checkSignin(text);
    }
     //m_curLen = text.length();
}


bool SigninDialog::searchSignin(int group)
{
    printf("searchSignin group: %d \n", group);
    QString db="SininMembs";
    switch(group){
       case GROUP_TNT:
           db="SininMembs";
           break;
       case GROUP_SPARK:
           db="SparkSigninMembs";
           break;
       case GROUP_COWK:
           db="CowkSigninMembs";
           break;
       case GROUP_CUBI:
           db="CubiSigninMembs";
           break;
     }

    //QSqlDatabase database = QSqlDatabase::database();
    QSqlQuery chkQuery(m_database);
    QString sqlStr;

    sqlStr = QString("select grade, gender,color from %1 where name = '%2'").arg(db).arg(m_barCode);

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return false;
    }

    if ( chkQuery.next() )
    {

        m_grade = chkQuery.value(0).toString();
        m_gender =  chkQuery.value(1).toString();
        m_color =  chkQuery.value(2).toString();
        if( group == GROUP_COWK) m_color = "brown";
        else if( group == GROUP_CUBI) m_color = "pink";

        findSignin();

        return true;

    }

    printf("not find in group:%d \n", group);
    return false;

}

void SigninDialog::onReceiveBarCode()
{
    printf("onReceiveBarCode:%s \n", qPrintable(m_barCode));

    m_signinState = 3;
   // m_timer->stop();


    m_group = GROUP_TNT;
    if(searchSignin(m_group)) {
        updateUI();

        ui->btnOKSignin->setFocus();
        return;
    }

    m_group = GROUP_SPARK;
    if(searchSignin(m_group)) {
         updateUI();
         ui->btnOKSignin->setFocus();
        return;
    }

    m_group = GROUP_COWK;
    if(searchSignin(m_group)) {
         updateUI();
         ui->cbSunday->setDisabled(true);
         ui->btnOKSignin->setFocus();
        return;
    }

    m_group = GROUP_CUBI;
    if(searchSignin(m_group)) {
         updateUI();
         ui->cbSunday->setDisabled(true);
         ui->btnOKSignin->setFocus();
        return;
    }

    m_signinState = 2;

    QMessageBox::critical(NULL, QObject::tr("Sign In Fail"), QString("The Name [%1] \n cannot be found").arg(m_barCode));
    updateUI();
    ui->leNameSignin->setFocus();
   // m_timer->start(100);


}
void SigninDialog::keyPressEvent(QKeyEvent* event)
{
    printf("keyPressEvent, state = %d \n", m_signinState);
    if( m_signinState == 3 ) return;

    /*
    if(event->key() == Qt::Key_Escape){
        if(isFullScreen ()){

            showMaximized();
        }
        else{
            showFullScreen();
        }
    }
*/

    QString newChar = event->text();

    if( newChar == "\n" || newChar == "\r"){
        QString name = ui->leNameSignin->text().trimmed();

        if(m_signinState == 0 &&name.length() == 0) return;

       // if( ! ui->cbBadge->isChecked())
        {

             m_barCode = name;
        }
       // m_barCode = m_barCode;

        printf("get a scan:%s \n", qPrintable(m_barCode));

        emit receivedBarCode();

    }
    else{
        if(newChar.isSimpleText() ){

            printf("simple text , state = %d \n", m_signinState);

            if(m_signinState == 0 || m_signinState == 2 ){

                ui->lbFeedback->hide();

                m_signinState = 1;
                //ui->cbBadge->setDisabled(true);
                updateUI();
            }

           // if( ui->cbBadge->isChecked())
            {
                //qDebug() << " simple text :" << newChar;
               // m_barCode = m_barCode + newChar;

                //ui->leNameSignin->setText(m_barCode);

                printf("[%s] \n",qPrintable(m_barCode));
            }
        }
        else{
            printf("not a simple text \n");
        }

    }
}

void SigninDialog::on_btnContinue_clicked()
{
    int group = m_group;

    while(group < GROUP_COWK ){
        group ++;
        if(searchSignin(group)) {

            m_group = group;
            return;
        }
    }

    QMessageBox::critical(NULL, QObject::tr("Sign In Fail"), m_barCode);

}


void SigninDialog::cbBadgeChange(int state)
{
    //return;
    printf("on_cbBadgeChange \n");

    if(state == Qt::Checked){

        ui->leNameSignin->setDisabled(true);
        //ui->lwNames->setDisabled(true);
        // m_timer->stop();
    }
    else{
        ui->leNameSignin->setDisabled(false);
        //ui->lwNames->setDisabled(false);
        // m_timer->start(1000);
         m_curLen = 0;

    }
}


void SigninDialog::clearSignin()
{
    printf("clearSignin");
    m_barCode = "";
     m_enter = "";
    m_curLen = 0;
    ui->leNameSignin->setText(m_barCode);
    ui->leNameSignin->setDisabled(false);

    ui->lwNames->clear();


    ui->leGradeSignin->setText(m_barCode);
    ui->leGenderSignin->setText(m_barCode);

    m_signinState = 0;

    //ui->cbBadge->setDisabled(false);

    //ui->cbBadge->setChecked(true);


    ui->cbSunday->setChecked(true);
    ui->cbUniform->setChecked(true);


    ui->cbLate->setDisabled(true);

    QTime curTime = QTime::currentTime();
    QTime lateTime = QTime::fromString("07:30 PM", "hh:mm AP");

    if(curTime < lateTime ){
        ui->cbLate->setChecked(false);
    }
    else{
        ui->cbLate->setChecked(true);
    }
    //ui->cbLate->setChecked(false);

    ui->lbColorSignin->setStyleSheet("QLabel { background-color : none;  }");

     ui->cbSunday->setDisabled(false);

    //ui->btnContinue->setDisabled(true);
     updateUI();

     ui->leNameSignin->setFocus();

}

void SigninDialog::on_btnOKSignin_clicked()
{
    printf("on_btnOKSignin_clicked : group=%d \n", m_group);

    if(  m_signinState != 3) return;

    emit sigGotSignin(m_barCode, m_color, m_group, ui->cbSunday->isChecked(), ui->cbUniform->isChecked(), ui->cbLate->isChecked(), ui->cbBadge->isChecked());

    clearSignin();

    //ui->lbFeedback->setPixmap(QPixmap::fromImage(m_imageGreen));

    ui->lbFeedback->show();


}

void SigninDialog::on_btnCancelSignin_clicked()
{

    clearSignin();
    //ui->lbFeedback->setPixmap(QPixmap::fromImage(m_imageRed));
    //ui->lbFeedback->show();
}

