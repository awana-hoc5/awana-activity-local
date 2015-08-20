#include "mainwindow.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QUrlQuery>
#endif

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QFileDialog>
#include <QTextStream>
#include <QDesktopServices>
#include <QSqlQuery>
#include <QNetworkCookieJar>
#include <QSqlRecord>
#include <QWaitCondition>
#include <QMutex>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QTime>

#include <QPainter>
#include <QKeyEvent>
#include <QSqlRecord>
#include <QSqlField>
#include <QAuthenticator>
#include <QSettings>

//#include "ui_mainwindow.h"

#include "signdialog.h"
#include "signindialog.h"
//#include "signoutdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    //ui(new Ui::MainWindow)
{
   setupUi(this);
   tbwSignInTnt->setColumnWidth(0,  120);
   tbwSignInSpark->setColumnWidth(0,  120);
   tbwSignInCowk->setColumnWidth(0,  120);
   tbwSignInCubi->setColumnWidth(0,  120);

    //m_barCode = "";

   //m_groupWindow = new GroupWindow();

   m_timer = new QTimer(this);

   connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

   connect(rbTnt, SIGNAL(toggled(bool)),  this, SLOT(on_rbTntChange(bool)));
   connect(rbSpark, SIGNAL(toggled(bool)),  this, SLOT(on_rbSparkChange(bool)));
   connect(rbCowk, SIGNAL(toggled(bool)),  this, SLOT(on_rbCowkChange(bool)));
   connect(rbCubi, SIGNAL(toggled(bool)),  this, SLOT(on_rbCubiChange(bool)));

   connect(rbTntOverview, SIGNAL(toggled(bool)),  this, SLOT(on_rbTntOverviewChange(bool)));
   connect(rbSparkOverview, SIGNAL(toggled(bool)),  this, SLOT(on_rbSparkOverviewChange(bool)));
   connect(rbCowkOverview, SIGNAL(toggled(bool)),  this, SLOT(on_rbCowkOverviewChange(bool)));
   connect(rbCubiOverview, SIGNAL(toggled(bool)),  this, SLOT(on_rbCubiOverviewChange(bool)));


   connect(tabWidget, SIGNAL(currentChanged(int)),  this, SLOT(on_tabCurrentChanged(int)));

   connect(leNameTntBook, SIGNAL(textChanged (const QString)),  this, SLOT(tntBookNameTextChanged(const QString)));
   connect(lwTntBook, SIGNAL(itemClicked  ( QListWidgetItem *  )),  this, SLOT(lwTntBookItemSelected(QListWidgetItem * )));
   connect(leSessionTntBook, SIGNAL(textChanged (const QString)),  this, SLOT(tbtBookTextChanged(const QString)));

   connect(leNameSpkBook, SIGNAL(textChanged (const QString)),  this, SLOT(spkBookNameTextChanged(const QString)));
   connect(lwSpkBook, SIGNAL(itemClicked  ( QListWidgetItem *  )),  this, SLOT(lwSpkBookItemSelected(QListWidgetItem * )));
   connect(leSessionSpkBook, SIGNAL(textChanged (const QString)),  this, SLOT(spkBookTextChanged(const QString)));



    m_httpStr="";
    m_numNewMemb = 0;
    m_numOldMemb = 0;
    m_timerCnt = 0;
    m_httpState = 0;

    m_loadSignDate = QDateTime::currentDateTime().addDays(-1);
    m_loadSignState = 0;
    m_signNum = 0;

    m_numSignout = 0;

    m_updateStr="";

    //m_deviceThread = NULL;
    m_initDB = 0;

    LoadInit();
    initDB();

    // tabWidget->setCurrentIndex(2);
    setSetting();
    checkSettings();


    lbTntBookOK->hide();
    lbSpkBookOK->hide();

    m_spkBookState = BOOK_STATE_IDLE;
    m_tntBookState = BOOK_STATE_IDLE;

}

MainWindow::~MainWindow()
{
    //delete ui;

}

void MainWindow::checkSettings()
{

    printf("checksettings:%d, %d,%d,%d \n",m_initDB, m_scanSignin, m_scanSignout, m_scanOther);
    int index ;
    int lastIndex = tabWidget->indexOf(tabConfig);


    if( m_scanSignin ){
        for(int i = 0; i < lastIndex; i++){
            tabWidget->setTabEnabled(i, false);
        }
        index = tabWidget->indexOf(tabSignin);
        tabWidget->setCurrentIndex(index);
        tabWidget->setTabEnabled(index, true);

    }
    /*
    else if(m_scanSignout ){

        for(int i = 0; i < lastIndex; i++){
            tabWidget->setTabEnabled(i, false);
        }
        index = tabWidget->indexOf(tabSignout);
        tabWidget->setCurrentIndex(index);
        tabWidget->setTabEnabled(index, true);

    }*/
    else{
        for(int i = 0; i < lastIndex; i++){
            tabWidget->setTabEnabled(i, true);
        }
        index = tabWidget->indexOf(tabSignin);
        tabWidget->setCurrentIndex(index);


    }

    if( m_initDB == 0 ){
        index = tabWidget->indexOf(tabInit);
        tabWidget->setTabEnabled(index, false);
    }
    else{
        index = tabWidget->indexOf(tabInit);
        tabWidget->setTabEnabled(index, true);
    }
}

void MainWindow::on_btnCfgChange_clicked()
{
    if(btnCfgChange->text().contains("Save") ){

        gbCfg->setDisabled(true);
        btnCfgChange->setText("Change Settings");
        SaveInit();

        checkSettings();

    }
    else{
        if(leCfgPwd->text().contains("hoc5!234")){
            gbCfg->setDisabled(false);
            btnCfgChange->setText("Save Settings");
            leCfgPwd->setText("");
            cbCfgInitDB->setDisabled(false);


        }
    }
}

void MainWindow::setSetting()
{

    if( m_scanSignin ){
        rbCfgScanSignin->setChecked(true);
        rbCfgScanSignout->setChecked(false);
        rbCfgOther->setChecked(false);

    }
    else if(m_scanSignout ){
        rbCfgScanSignin->setChecked(false);
        rbCfgScanSignout->setChecked(true);
        rbCfgOther->setChecked(false);
    }
    else {
        rbCfgScanSignin->setChecked(false);
        rbCfgScanSignout->setChecked(false);
        rbCfgOther->setChecked(true);
    }

    if( m_initDB == 0 ){
        cbCfgInitDB->setChecked(false);
    }
    else{
        cbCfgInitDB->setChecked(true);
    }
}

void MainWindow::getSetting()
{
    m_initDB = 0;
    if( cbCfgInitDB->isChecked()){
        m_initDB = 1;
    }

    if( rbCfgOther->isChecked()){
        m_scanOther = 1;
        m_scanSignin = 0 ;
        m_scanSignout = 0 ;
    }
    else if( rbCfgScanSignin->isChecked()){
        m_scanOther = 0;
        m_scanSignin = 1 ;
        m_scanSignout = 0 ;
    }
    else{
        m_scanOther = 0;
        m_scanSignin = 0 ;
        m_scanSignout = 1 ;
    }

}

void MainWindow::SaveInit()
{

    getSetting();
    QString ini_fn = QStandardPaths::writableLocation(QStandardPaths::DataLocation)  + QDir::separator()  + "csv_gui.ini";;


    //ini_fn = QDir::toNativeSeparators(ini_fn);
    QSettings settings(ini_fn, QSettings::IniFormat);

    settings.beginGroup("DATA");

    settings.setValue("init_DB", m_initDB );

    settings.setValue("scan_Signin", m_scanSignin  );

    settings.setValue("scan_Signout", m_scanSignout  );

    settings.setValue("scan_other", m_scanSignout  );


    settings.endGroup();
    settings.sync();

   printf("save init:%d, %d, %d, %d \n",m_initDB, m_scanSignin, m_scanSignout, m_scanSignout);
}

void MainWindow::LoadInit()
{

    QCoreApplication::setOrganizationName("Hoc5");
    QCoreApplication::setOrganizationDomain("www.hoc5.org");
    QCoreApplication::setApplicationName("AWA_MAINT");

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    QString dbPath = QDesktopServices::storageLocation ( QDesktopServices::DataLocation );
#else
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    QString orgPath;

    lbStatus->setText(dbPath);
    printf("dbPath=%s \n", qPrintable(dbPath));
    //int index = dbPath.lastIndexOf(QDir::separator());
    int index = dbPath.lastIndexOf("AWA_MAINT");
     if( index > 0 ){
        orgPath = dbPath.left(index -1 );
    }
    else{

        QMessageBox::critical(NULL, QObject::tr("Database path Error"), dbPath);
        return ;

    }
    if(!QDir(orgPath).exists()){
       if(!QDir().mkdir(orgPath)){
           QMessageBox::critical(NULL, QObject::tr("create org path Error"), orgPath);
       }
    }
    if(!QDir(dbPath).exists()){

       if(!QDir().mkdir(dbPath)){
           QMessageBox::critical(NULL, QObject::tr("create path Error"), dbPath);

       }
    }

    QString imgPath = dbPath +  QDir::separator() + "image";

    if(!QDir(imgPath).exists()){

         if(!QDir().mkdir(imgPath)){
             printf("mkdir failed: %s \n", qPrintable(imgPath));
             QMessageBox::critical(NULL, QObject::tr("create path Error"), imgPath);

         }
         else{
             printf("mkdir OK: %s \n", qPrintable(imgPath));
         }
      }
      else{
           printf("exist: %s \n", qPrintable(imgPath));
      }



    QString iniFn = dbPath + QDir::separator()  + "csv_gui.ini";


    printf("ini file path:%s\n", qPrintable(iniFn));
     //ini_fn = QDir::toNativeSeparators(ini_fn);

     QSettings settings(iniFn, QSettings::IniFormat);
     QVariant value;

     settings.sync();


     settings.beginGroup("DATA");

     value = settings.value("init_DB");
     if ( value.isValid() )
     {
        m_initDB = value.toInt();
     }
     else{
         printf("no init_DB\n");
        m_initDB = 0;
     }

     value = settings.value("scan_Signin");
     if ( value.isValid() )
     {
        m_scanSignin = value.toInt();
     }
     else{
          printf("no scan_Signin \n");
        m_scanSignin = 0;
     }

     value = settings.value("scan_Signout");
     if ( value.isValid() )
     {
        m_scanSignout = value.toInt();
     }
     else{
        m_scanSignout = 0;
     }

     value = settings.value("scan_other");
     if ( value.isValid() )
     {
        m_scanOther = value.toInt();
     }
     else{
        m_scanOther = 0;
     }

     settings.endGroup();

}


int MainWindow::initDB()
{

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    QString dbPath = QDesktopServices::storageLocation ( QDesktopServices::DataLocation );
#else
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif

    QString db_name = dbPath + "/awana.db";


     // open db connection
     m_database = QSqlDatabase::addDatabase("QSQLITE");

     m_database.setDatabaseName(db_name);

     int rtn = m_database.open();

     if( !rtn )
     {
         printf("DB Open failed \n");
         return -1;
     }

     QStringList list = m_database.tables( QSql::Tables);
     int numOfTable = list.size();
     printf( "table list size:%d \n", numOfTable);

     if( numOfTable == 0 ){ //empty db
         printf( "new db with empty tables \n");

         QSqlQuery query(m_database);

        // QString queryCStr = QString("CREATE TABLE NewMembs (id INTEGER PRIMARY KEY,  name text, grade text, gender text)");

         QString queryCStr;

         queryCStr = QString("CREATE TABLE NewMembs ( name text, grade text, gender text)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE OldMembs (id INTEGER PRIMARY KEY, name text, grade text, gender text,  color text, active text, issignin integer, islate integer, sundaycnt integer, latecnt integer, attendcnt integer, uniformcnt integer, badgecnt integer)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE SininMembs (name text, grade text, gender text, color text, signin text, sunday text,late text,uniform text, badge text)");
         query.exec(queryCStr);



         queryCStr = QString("CREATE TABLE SparkNewMembs (name text, grade text, gender text)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE SparkOldMembs (id INTEGER PRIMARY KEY,name text, grade text, gender text, color text, active text, issignin integer, islate integer, sundaycnt integer, latecnt integer, attendcnt integer, uniformcnt integer, badgecnt integer)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE SparkSigninMembs (name text, grade text, gender text, color text,  signin text, sunday text,late text,uniform text, badge text)");
         query.exec(queryCStr);


         queryCStr = QString("CREATE TABLE CubiNewMembs (name text, grade text, gender text)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE CubiOldMembs (id INTEGER PRIMARY KEY, name text, grade text, gender text, color text, active text, issignin integer, islate integer, sundaycnt integer, latecnt integer, attendcnt integer, uniformcnt integer, badgecnt integer)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE CubiSigninMembs (name text, grade text, gender text, color text,  signin text, sunday text,late text,uniform text, badge text)");
         query.exec(queryCStr);



         queryCStr = QString("CREATE TABLE CowkNewMembs (name text, grade text, gender text)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE CowkOldMembs (id INTEGER PRIMARY KEY, name text, grade text, gender text, color text, active text, issignin integer, islate integer, sundaycnt integer, latecnt integer, attendcnt integer, uniformcnt integer, badgecnt integer)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE CowkSigninMembs (name text, grade text, gender text, color text,  signin text, sunday text,late text,uniform text, badge text)");
         query.exec(queryCStr);


         queryCStr = QString("CREATE TABLE SigninStatus (datetime timestamp, loadstatus integer, num2bUpd integer)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE CowkSignout (name text, datetime timestamp)");
         query.exec(queryCStr);

         queryCStr = QString("CREATE TABLE BookReport (name text, club text, grade text, record text)");
         query.exec(queryCStr);


         queryCStr = QString("CREATE TABLE NewSignin (name text, grade text, gender text,  color text, sunday text, book text, updb text)"); //
         if(!query.exec(queryCStr)){
             qDebug() <<  " db failed: " << queryCStr;

         }


         query.finish();


     }
     else{
         showMember(MEMBER_NEW); //new member
         showMember(MEMBER_OLD); //old member

         retrieveSignin(false); //new singnin no db updated

         if(0)
         {
             QSqlQuery query(m_database);

             QString queryCStr;

             queryCStr = QString("CREATE TABLE CubiNewMembs (name text, grade text, gender text)");
             query.exec(queryCStr);

             queryCStr = QString("CREATE TABLE CubiOldMembs (name text, grade text, gender text, active text)");
             query.exec(queryCStr);

             queryCStr = QString("CREATE TABLE CubiSigninMembs (name text, grade text, gender text, color text,  signin text, sunday text,late text,uniform text, badge text)");
             query.exec(queryCStr);

             query.finish();
        }

     }


     QSqlQuery query(m_database);
     QString queryCStr;

     queryCStr = QString("SELECT * from SigninStatus ");
     query.exec(queryCStr);
     if(!query.exec(queryCStr)){
         printf(" db failed: %s \n " , qPrintable(queryCStr));

     }
     else{
         if ( query.next() )
         {
             QString str = query.value(0).toString();

             QString format = parseFormatStr(str);

             if(format.isEmpty() ){
                 printf("empty time str\n");
                 m_loadSignDate = QDateTime::currentDateTime().addDays(-1);
                 m_loadSignState = 0;
                 m_signNum = 0;
                 btnStartSign->setDisabled(true);
                 btnSigninUpd->setDisabled(true);
                 btnLoadSignin->setDisabled(true);

             }
             else{

                 QDateTime dt = QDateTime::fromString(str, format);

                 QDate tm = dt.date();

                 //printf( "date in db [%s], curr :%s \n",qPrintable(str),qPrintable(str));

                 if( tm != QDate::currentDate()){
                     printf("old signin date \n");
                     m_loadSignState = 0;
                     m_signNum = 0;
                     btnStartSign->setDisabled(true);
                     btnSigninUpd->setDisabled(true);
                     btnLoadSignin->setDisabled(true);

                 }
                 else{

                     printf("signin loaded \n");

                    m_loadSignState = query.value(1).toInt();
                    m_signNum = query.value(2).toInt();
                    leNumSignin->setText(QString("%1").arg(m_signNum));

                    switch(m_loadSignState){
                    case 0:
                        btnStartSign->setDisabled(true);
                        btnSigninUpd->setDisabled(true);
                        btnLoadSignin->setDisabled(true);
                        break;
                    case 1: //game started
                        btnStartSign->setDisabled(true);
                        btnSigninUpd->setDisabled(true);
                       // btnLoadSignin->setDisabled(true);
                        break;
                    case 2: //signin data updated
                        break;
                    case 3: // in the middle of signin

                        btnLoadSignin->setDisabled(true);
                        break;

                    }

                 }

             }

         }
         else{
             printf("status db is empty \n");
             m_loadSignDate = QDateTime::currentDateTime().addDays(-1);
             m_loadSignState = 0;
             m_signNum = 0;
             btnStartSign->setDisabled(true);
             btnSigninUpd->setDisabled(true);
             btnLoadSignin->setDisabled(true);


             queryCStr = QString("INSERT INTO SigninStatus (datetime , loadstatus , num2bUpd ) VALUES (\'");

             queryCStr += m_loadSignDate.toString("yyyy-MM-dd hh:mm:ss");
             queryCStr += "\', \'";
             queryCStr += QString("%1").arg(m_loadSignState);
             queryCStr += "\', \'";
             queryCStr += QString("%1").arg(m_signNum);;

             queryCStr += "\')";

             if (!query.exec(queryCStr))
             {
                 printf("get error:%s \n", qPrintable(queryCStr));
             }

         }
         query.finish();
     }
     return 0;
}

QString MainWindow::parseFormatStr(const QString& dt)
{
    //dt: 2014-1-01 00:00:00
    QString fm2 = "yyyy-MM-dd hh:mm:ss";
    QString fm1 = "yyyy-M-dd hh:mm:ss";
    QStringList sl1 = dt.split(" ");
    if (sl1.size() <= 0 ) return "";
    QStringList sl2 = sl1.at(0).split("-");
    if (sl2.size() <= 1 ) return "";
    QString m = sl2.at(1);
    int len = m.length();
    if( len == 1)
        return fm1;
    else return fm2;


}
void MainWindow::deleteTable(int old)
{
    QSqlQuery query(m_database);

    QString queryStr ;

    if( old == MEMBER_OLD ){
        if(rbTnt->isChecked()){
            queryStr= QString("DELETE FROM OldMembs");
        }
        else if(rbSpark->isChecked()){
            queryStr= QString("DELETE FROM SparkOldMembs");
        }
        else  if(rbCowk->isChecked()){
             queryStr= QString("DELETE FROM CowkOldMembs");

        }
        else{
              queryStr= QString("DELETE FROM CubiOldMembs");
        }
        if (!query.exec(queryStr))
        {
            printf("delete failed:%s \n", qPrintable(queryStr));
        }
    }
    else{
        queryStr= QString("DELETE FROM NewMembs");

        if (!query.exec(queryStr))
        {
            printf("delete failed :%s \n", qPrintable(queryStr));
        }

        queryStr= QString("DELETE FROM SparkNewMembs");

        if (!query.exec(queryStr))
        {
            printf("delete failed:%s \n", qPrintable(queryStr));
        }

        queryStr= QString("DELETE FROM CowkNewMembs");

        if (!query.exec(queryStr))
        {
            printf("delete failed:%s \n", qPrintable(queryStr));
        }
        queryStr= QString("DELETE FROM CubiNewMembs");

        if (!query.exec(queryStr))
        {
            printf("delete failed:%s \n", qPrintable(queryStr));
        }
    }

}


void MainWindow::showMember(int old)
{

    printf("showMember :%d\n", old);
    if( old == MEMBER_OLD){

        if(rbTnt->isChecked()){
             printf("get data from OldMembs \n");
             m_tableModel.setQuery("SELECT name, grade, gender FROM OldMembs where active='yes'", m_database);
             m_inactTableModel.setQuery("SELECT * FROM OldMembs where active='no'", m_database);


        }
        else if(rbSpark->isChecked()){
            printf("get data from SparkOldMembs \n");
            m_tableModel.setQuery("SELECT name, grade, gender  FROM SparkOldMembs where active='yes'", m_database);
            m_inactTableModel.setQuery("SELECT * FROM SparkOldMembs where active='no'", m_database);

        }
        else  if(rbCowk->isChecked()) {
            printf("get data from CowkOldMembs \n");
            m_tableModel.setQuery("SELECT name, grade, gender  FROM CowkOldMembs where active='yes'", m_database);
            m_inactTableModel.setQuery("SELECT * FROM CowkOldMembs where active='no'", m_database);

        }
        else{
            printf("get data from CubiOldMembs \n");
            m_tableModel.setQuery("SELECT name, grade, gender  FROM CubiOldMembs where active='yes'", m_database);
            m_inactTableModel.setQuery("SELECT * FROM CubiOldMembs where active='no'", m_database);

        }
         //m_tableModel.setQuery( QString("select * from users where operator_level >= '%1'").arg(m_nOpLevel), m_database);
         m_tableModel.setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
         m_tableModel.setHeaderData(1, Qt::Horizontal, QObject::tr("Grade"));
         m_tableModel.setHeaderData(2, Qt::Horizontal, QObject::tr("Gender"));


        // int rCnt = m_tableModel.rowCount();
        // printf( "table row count = %d \n", rCnt);


         m_sortedModel.setSourceModel(&m_tableModel);

         tbvOldMemb->setModel(&m_sortedModel);
         tbvOldMemb->show();

         tbvOldMemb->setColumnWidth(0, 150);
         tbvOldMemb->setColumnWidth(1, 100);
         tbvOldMemb->setColumnWidth(2, 100);

         while(m_tableModel.canFetchMore())
                m_tableModel.fetchMore();

         m_numOldMemb = m_tableModel.rowCount();

         printf("m_numOldMemb = %d \n", m_numOldMemb);


         m_inactTableModel.setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
         m_inactTableModel.setHeaderData(1, Qt::Horizontal, QObject::tr("Grade"));
         m_inactTableModel.setHeaderData(2, Qt::Horizontal, QObject::tr("Gender"));

         m_inactSortedModel.setSourceModel(&m_inactTableModel);

         tbvInactive->setModel(&m_inactSortedModel);
         tbvInactive->show();

         tbvInactive->setColumnWidth(0, 150);
         tbvInactive->setColumnWidth(1, 100);
         tbvInactive->setColumnWidth(2, 100);


    }
    else{

        if(rbTnt->isChecked()){
            printf("get data from NewMembs \n");
            m_newTableModel.setQuery("SELECT name, grade, gender  FROM NewMembs", m_database);
        }
        else if(rbSpark->isChecked()){
            printf("get data from SparkNewMembs \n");
            m_newTableModel.setQuery("SELECT name, grade, gender  FROM SparkNewMembs", m_database);

        }
        else  if(rbCowk->isChecked()){
            printf("get data from CowkNewMembs \n");
            m_newTableModel.setQuery("SELECT name, grade, gender  FROM CowkNewMembs", m_database);

        }
        else{
            printf("get data from CubiNewMembs \n");
            m_newTableModel.setQuery("SELECT name, grade, gender  FROM CubiNewMembs", m_database);

        }
        m_newTableModel.setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
        m_newTableModel.setHeaderData(1, Qt::Horizontal, QObject::tr("Grade"));
        m_newTableModel.setHeaderData(2, Qt::Horizontal, QObject::tr("Gender"));


       // int rCnt = m_tableModel.rowCount();
       // printf( "table row count = %d \n", rCnt);

        m_newSortedModel.setSourceModel(&m_newTableModel);

        tbvNewMemb->setModel(&m_newSortedModel);
        tbvNewMemb->show();

        tbvNewMemb->setColumnWidth(0, 150);
        tbvNewMemb->setColumnWidth(1, 100);
        tbvNewMemb->setColumnWidth(2, 100);

        while(m_newTableModel.canFetchMore())
               m_newTableModel.fetchMore();

        m_numNewMemb = m_newTableModel.rowCount();
        printf("m_numNewMemb = %d \n", m_numNewMemb);

    }

     checkDup( old);
}


void MainWindow::showMemberOverview()
{

    printf("showMemberOverview \n");


        if(rbTntOverview->isChecked()){
             printf("get data from OldMembs \n");
             m_tableModel.setQuery("SELECT name, grade, gender, attendcnt, latecnt, sundaycnt, uniformcnt, badgecnt, issignin, islate, color FROM OldMembs where active='yes'", m_database);


        }
        else if(rbSparkOverview->isChecked()){
            printf("get data from SparkOldMembs \n");
            m_tableModel.setQuery("SELECT name, grade, gender, attendcnt, latecnt, sundaycnt, uniformcnt, badgecnt, issignin, islate, color FROM SparkOldMembs where active='yes'", m_database);

        }
        else  if(rbCowkOverview->isChecked()) {
            printf("get data from CowkOldMembs \n");
            m_tableModel.setQuery("SELECT name, grade, gender, attendcnt, latecnt, sundaycnt, uniformcnt, badgecnt, issignin, islate, color FROM CowkOldMembs where active='yes'", m_database);

        }
        else{
            printf("get data from CubiOldMembs \n");
            m_tableModel.setQuery("SELECT name, grade, gender, attendcnt, latecnt, sundaycnt, uniformcnt, badgecnt, issignin, islate, color FROM CubiOldMembs where active='yes'", m_database);

        }
         //m_tableModel.setQuery( QString("select * from users where operator_level >= '%1'").arg(m_nOpLevel), m_database);
         m_tableModel.setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
         m_tableModel.setHeaderData(1, Qt::Horizontal, QObject::tr("Grade"));
         m_tableModel.setHeaderData(2, Qt::Horizontal, QObject::tr("Gender"));
         m_tableModel.setHeaderData(3, Qt::Horizontal, QObject::tr("Attend"));
         m_tableModel.setHeaderData(4, Qt::Horizontal, QObject::tr("Late"));
         m_tableModel.setHeaderData(5, Qt::Horizontal, QObject::tr("Sunday"));
         m_tableModel.setHeaderData(6, Qt::Horizontal, QObject::tr("Uniform"));
         m_tableModel.setHeaderData(7, Qt::Horizontal, QObject::tr("Badge"));
         m_tableModel.setHeaderData(8, Qt::Horizontal, QObject::tr("Signin"));
         m_tableModel.setHeaderData(9, Qt::Horizontal, QObject::tr("Late"));
         m_tableModel.setHeaderData(10, Qt::Horizontal, QObject::tr("Color"));


        // int rCnt = m_tableModel.rowCount();
        // printf( "table row count = %d \n", rCnt);


         m_sortedModel.setSourceModel(&m_tableModel);

         tbvOverview->setModel(&m_sortedModel);
         tbvOverview->show();

         tbvOverview->setColumnWidth(0, 150);
         tbvOverview->setColumnWidth(1, 100);
         tbvOverview->setColumnWidth(2, 100);
         tbvOverview->setColumnWidth(3, 50);
         tbvOverview->setColumnWidth(4, 50);
         tbvOverview->setColumnWidth(5, 50);
         tbvOverview->setColumnWidth(6, 50);
         tbvOverview->setColumnWidth(7, 50);
         tbvOverview->setColumnWidth(8, 50);
         tbvOverview->setColumnWidth(9, 50);
         tbvOverview->setColumnWidth(10, 50);

         while(m_tableModel.canFetchMore())
                m_tableModel.fetchMore();


         int red = getColorNumber(1);
         lbTeamRed->setText(QString("Red: %1").arg(red));

         int green = getColorNumber(2);
         lbTeamGreen->setText(QString("Green: %1").arg(green));

         int blue = getColorNumber(3);
         lbTeamBlue->setText(QString("Blue: %1").arg(blue));

         int yellow = getColorNumber(4);
         lbTeamYellow->setText(QString("Yellow: %1").arg(yellow));
         printf("Red: %d, Green: %d, Blue: %d, Yellow:%d \n", red, green, blue, yellow);


}

void MainWindow::checkDup(int old)
{

    if( old == MEMBER_OLD ){

        int cnt = tbwOldDup->rowCount();

        printf("checkDup total old cnt = %d, dup cnt = %d \n", m_numOldMemb, cnt);

        for( int i = 0; i< cnt ; i ++ ){
            tbwOldDup->removeRow ( cnt - i -1 );
        }


        for (int i = 0; i< m_numOldMemb; i ++){

            QSqlRecord recOrig = m_tableModel.record(i);

            QString name = recOrig.value("name").toString();
            QString Gen = recOrig.value("gender").toString();
            QString Grade = recOrig.value("grade").toString();

            for( int j = i+1; j<  m_numOldMemb; j++ ){

                QSqlRecord recNext = m_tableModel.record(j);

                QString dupName = recNext.value("name").toString();
                QString dupGen = recNext.value("gender").toString();
                QString dupGrad = recNext.value("grade").toString();

                if( name.contains(dupName)){

                    printf("dup gen= %s, gra=%s \n",qPrintable(dupGen), qPrintable(dupGrad));

                    tbwOldDup->setUpdatesEnabled(false);

                    int rowCnt = tbwOldDup->rowCount();
                    tbwOldDup->setRowCount(rowCnt + 1);

                    tbwOldDup->setItem(rowCnt, 0, new QTableWidgetItem(dupName));
                    tbwOldDup->setItem(rowCnt, 1, new QTableWidgetItem(dupGrad));
                    tbwOldDup->setItem(rowCnt, 2, new QTableWidgetItem(dupGen));

                    tbwOldDup->setUpdatesEnabled(true);

                }
            }
        }
    }
    else{

        int cnt = tbwNewDup->rowCount();

        printf("checkDup total new cnt = %d, dup cnt = %d \n", m_numNewMemb, cnt);

        for( int i = 0; i< cnt ; i ++ ){
            tbwNewDup->removeRow ( cnt -1 - i );
        }

        for (int i = 0; i< m_numNewMemb; i ++){

            QSqlRecord recOrig = m_newTableModel.record(i);

            QString name = recOrig.value("name").toString(); //nameItem->text();
            QString Gen = recOrig.value("gender").toString();
            QString Grade = recOrig.value("grade").toString();

           // printf("i = %d, name = %s \n", i, qPrintable(name));

            for( int j = i+1; j<  m_numNewMemb; j++ ){

                QSqlRecord recNext = m_newTableModel.record(j);

                QString dupName = recNext.value("name").toString();
                QString dupGen = recNext.value("gender").toString();
                QString dupGrad = recNext.value("grade").toString();


                if( name.contains(dupName)){

                    //printf("dupName = %s \n", qPrintable(dupName));
                   // printf("dup name=%s, gen= %s, gra=%s \n",qPrintable(dupName), qPrintable(dupGen), qPrintable(dupGrad));

                    tbwNewDup->setUpdatesEnabled(false);

                    int rowCnt = tbwNewDup->rowCount();
                    tbwNewDup->setRowCount(rowCnt + 1);

                    tbwNewDup->setItem(rowCnt, 0, new QTableWidgetItem(dupName));
                    tbwNewDup->setItem(rowCnt, 1, new QTableWidgetItem(dupGrad));
                    tbwNewDup->setItem(rowCnt, 2, new QTableWidgetItem(dupGen));

                    tbwNewDup->setUpdatesEnabled(true);

                }
            }

        }
    }
}


void MainWindow::foundNewMemb()
{
    int found = 0;
    int cnt = tbwAdded->rowCount();

    for( int i = 0; i< cnt ; i ++ ){
        tbwAdded->removeRow ( cnt - 1 - i );
    }

    printf("foundNewMemb :%d \n",m_numNewMemb);

    for (int i = 0; i< m_numNewMemb; i ++){

        QSqlRecord recOrig = m_newTableModel.record(i);

        QString name = recOrig.value("name").toString();
        QString Gen = recOrig.value("gender").toString();
        QString Grade = recOrig.value("grade").toString();

        //printf("from new table model (%d): %s, %s,%s \n ");

        QSqlQuery chkQuery(m_database);
        QString sqlStr;
        if(rbTnt->isChecked()){
            //printf("get from OldMembs \n");
            sqlStr = QString("select * from OldMembs where name = '%1' and active='yes'").arg(name);
        }
        else if(rbSpark->isChecked() ){
            //printf("get from SparkOldMembs :%s \n", qPrintable(name));
            sqlStr = QString("select * from SparkOldMembs where name = '%1' and active='yes'").arg(name);

        }
        else if(rbCowk->isChecked() ){
            //printf("get from SparkOldMembs :%s \n", qPrintable(name));
            sqlStr = QString("select * from CowkOldMembs where name = '%1' and active='yes'").arg(name);

        }
        else{
            sqlStr = QString("select * from CubiOldMembs where name = '%1' and active='yes'").arg(name);

        }

        if(!chkQuery.exec(sqlStr))
        {
            printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        }


        if (chkQuery.isActive()) {

            while ( chkQuery.next() ) {
                found = 1;
                break;
            }
        }
        //chkQuery.finish();

        if( found == 1) {
             //printf("old memb:%s \n", qPrintable(name));

             found = 0;
            continue;
        }

        printf("new  memb:%s \n", qPrintable(name));

        tbwAdded->setUpdatesEnabled(false);

        int rowCnt = tbwAdded->rowCount();
        tbwAdded->setRowCount(rowCnt + 1);

        tbwAdded->setItem(rowCnt, 0, new QTableWidgetItem(name));
        tbwAdded->setItem(rowCnt, 1, new QTableWidgetItem(Grade));
        tbwAdded->setItem(rowCnt, 2, new QTableWidgetItem(Gen));

        tbwAdded->setUpdatesEnabled(true);

       // tbwAdded->resizeColumnsToContents();

    }
}

void MainWindow::foundRemoveMemb()
{
    int found = 0;
    int cnt = tbwRemove->rowCount();
    printf("foundRemoveMemb cnt = %d \n", cnt);

    for( int i = 0; i< cnt ; i ++ ){
        tbwRemove->removeRow ( cnt- i -1 );
    }


    for (int i = 0; i< m_numOldMemb; i ++){

        QSqlRecord recOrig = m_tableModel.record(i);

        QString name = recOrig.value("name").toString();
        QString Gen = recOrig.value("gender").toString();
        QString Grade = recOrig.value("grade").toString();


        QSqlQuery chkQuery(m_database);
        QString sqlStr;
        if(rbTnt->isChecked()){
            sqlStr = QString("select * from NewMembs where name = '%1'").arg(name);
        }
        else if(rbSpark->isChecked()){
            sqlStr = QString("select * from SparkNewMembs where name = '%1'").arg(name);

        }
        else if(rbCowk->isChecked()){
            sqlStr = QString("select * from CowkNewMembs where name = '%1'").arg(name);

        }
        else{
            sqlStr = QString("select * from CubiNewMembs where name = '%1'").arg(name);

        }

        if(!chkQuery.exec(sqlStr))
        {
            printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        }


        if (chkQuery.isActive()) {

            while ( chkQuery.next() ) {
                found = 1;
                break;
            }
        }
        //chkQuery.finish();

        if( found == 1) {
             //printf("new memb:%s \n", qPrintable(name));

             found = 0;
            continue;
        }

        //printf("remove  memb:%s \n", qPrintable(name));

        tbwRemove->setUpdatesEnabled(false);

        int rowCnt = tbwRemove->rowCount();
        tbwRemove->setRowCount(rowCnt + 1);

        tbwRemove->setItem(rowCnt, 0, new QTableWidgetItem(name));
        tbwRemove->setItem(rowCnt, 1, new QTableWidgetItem(Grade));
        tbwRemove->setItem(rowCnt, 2, new QTableWidgetItem(Gen));

        tbwRemove->setUpdatesEnabled(true);

       // tbwAdded->resizeColumnsToContents();

    }
}

/*
void MainWindow::httpPostNewMemb(int op )  //2 or 3
{

    QString urlStr;

    if(op < OPCODE_TNT_NEW_SIGNIN ){ //4
        if(rbTnt->isChecked()){
            if(op == OPCODE_ADD_NEW){
                urlStr = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocProcess";
            }
            else if(op == OPCODE_INACT_MEMBER ){
                urlStr = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocDeact";
            }
        }
        else if(rbSpark->isChecked()){
            if(op == OPCODE_ADD_NEW){
                urlStr = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocProcess";
            }
            else if(op == OPCODE_INACT_MEMBER ){
                urlStr = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocDeact";
            }
        }
        else if(rbCowk->isChecked()){
            if(op == OPCODE_ADD_NEW){
                urlStr = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocAssistProc";
            }
            else if(op == OPCODE_INACT_MEMBER ){
                urlStr = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocAssistDeact";
            }
        }
        else{
            if(op == OPCODE_ADD_NEW){
                urlStr = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocCubiProc";
            }
            else if(op == OPCODE_INACT_MEMBER ){
                urlStr = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocCubiDeact";
            }
        }
    }
    else{
        if(op == OPCODE_TNT_NEW_SIGNIN){
            urlStr = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocNewSignin";
        }
        else if(op == OPCODE_SPARK_NEW_SIGNIN ){
            urlStr = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocNewSignin";

        }
        else{
            urlStr = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocAssistUpd";

        }
    }

}


void MainWindow::httpGetExitMemb()
{


    if(rbTnt->isChecked()){
        url = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDoc";

    }
    else if(rbSpark->isChecked() ){
        url = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDoc";

    }
    else if(rbCowk->isChecked()) {
        url = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocAssist";

    }
    else{
        url = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocCubi";

    }

}
*/




void MainWindow::onTimeout()
{

    m_timerCnt++;

    if( m_timerCnt > 3) {

        m_timerCnt= 0;
    }
}

void  MainWindow::on_btnLocal_clicked()
{

    int sparkState = 0;
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    QString folder = QDesktopServices::storageLocation ( QDesktopServices::DocumentsLocation );
#else
    QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif

    QString fileName = QFileDialog::getOpenFileName(this, tr("Get Spreadsheet CSV File"), folder, tr("CSV Files (*.csv)"));

    if( fileName.isNull()) return;

     deleteTable(MEMBER_NEW);
     showMember(MEMBER_NEW);

    QString gender="Boy";
    QString grade = "3rd";
    QString name = "Leon Xu";
    QString lastN="Xu";
    QString firstN ="Leon";

    QFile fileCsv(fileName) ;
    if ( fileCsv.open(QIODevice::ReadOnly) )  {

        QTextStream in(&fileCsv);
        QString line="";
        QStringList itemList;
        int len= 0;
        int cnt = 0;

        if( rbCowk->isChecked()){

            while ( !in.atEnd() )
            {
                cnt++;
                line = in.readLine() ;
                if( cnt == 1){
                    continue;

                }
                //printf("%s \n", qPrintable(line));

                itemList = line.split(",");
                len = itemList.size();

                if(len < 3) {
                    printf("< 3??");
                    continue;
                }

                if(itemList[5].trimmed() != "yes" ){
                    printf("!= yes ?? :%s \n", qPrintable(itemList[2]));
                    continue;
                }
                if(itemList[11].contains("Fe") ){
                    gender="Girl";
                }
                else {
                    gender="Boy";
                }

                if(itemList[4].contains("6-") ){

                     grade = "6th";
                }
                else if(itemList[4].contains("5-") ){
                    grade = "5th";
                }
                else if(itemList[4].contains("4-") ){
                    grade = "4th";
                }
                else if(itemList[4].contains("3-") ){
                    grade = "3rd";
                }
                else if(itemList[4].contains("2-") ){
                    grade = "2nd";
                }
                else if(itemList[4].contains("1-") ){
                    grade = "1st";
                }
                else if(itemList[4].contains("0K-") ){
                    grade = "Kin";
                }
                else if(itemList[4].contains("Cub") ){
                    grade = "Cub";
                }

                lastN = itemList[8];
                firstN = itemList[9];

                name = firstN.trimmed() + ' ' + lastN.trimmed();


                QSqlQuery query(m_database);
                QString queryStr;

                printf("cowk: %s, %s, %s \n", qPrintable(name), qPrintable(grade), qPrintable(gender));

                queryStr = QString("INSERT INTO CowkNewMembs (name , grade , gender ) VALUES (\'");

                queryStr += name.trimmed();
                queryStr += "\', \'";
                queryStr += grade;
                queryStr += "\', \'";
                queryStr += gender;

                queryStr += "\')";

                if (!query.exec(queryStr))
                {
                    printf("get error:%s \n", qPrintable(queryStr));
                }

            }


        }
        else{

            while ( !in.atEnd() )
            {
                cnt++;
                line = in.readLine() ;
                //printf("%s \n", qPrintable(line));


                if( cnt == 1){
                    continue;

                }
                itemList = line.split(",");
                len = itemList.size();
                if(itemList[0].isEmpty() ){
                    continue;
                }

                if(len < 3) continue;

                if(itemList[0].contains("6th") ){

                        grade = "6th";
                        sparkState = 0;
                }
                else if(itemList[0].contains("5th") )
                {
                        grade = "5th";
                        sparkState = 0;
                }
                else if(itemList[0].contains("4th") )
                {
                        grade = "4th";
                        sparkState = 0;

                }
                else if(itemList[0].contains("3rd") )
                {
                        grade = "3rd";
                        sparkState = 0;
                }
                else if(itemList[0].contains("2nd") )
                {
                        sparkState = 1;
                        //continue;

                        grade = "2nd";
                }
                else if(itemList[0].contains("1st") )
                {
                        sparkState = 1;

                        grade = "1st";

                }
                else if(itemList[0].contains("Kind") )
                {
                        sparkState = 1;
                        grade = "Kin";

                }
                else if(itemList[0].contains("Cubb") )
                {
                    sparkState = 2;
                    grade = "Cub";
                    continue;
                }


                if(itemList[0].contains("Boy")){
                    gender="Boy";

                    continue;
                }
                else if(itemList[0].contains("Girl") ){
                    gender="Girl";

                    continue;
                }

                if(sparkState == -1) continue;

                lastN = itemList[1];
                firstN = itemList[2];

                if(sparkState == 2){
                    gender= itemList[4];
                    if(gender.contains("F")){
                        gender="Girl";
                    }
                    else{
                        gender="Boy";
                    }
                }

                name = firstN.trimmed() + ' ' + lastN.trimmed();


                QSqlQuery query(m_database);
                QString queryStr;
                if(sparkState == 0 ){
                    queryStr = QString("INSERT INTO NewMembs (name , grade , gender ) VALUES (\'");
                }
                else if(sparkState == 1 ){
                    queryStr = QString("INSERT INTO SparkNewMembs (name , grade , gender ) VALUES (\'");

                }
                else{
                    queryStr = QString("INSERT INTO CubiNewMembs (name , grade , gender ) VALUES (\'");

                }

                queryStr += name.trimmed();
                queryStr += "\', \'";
                queryStr += grade;
                queryStr += "\', \'";
                queryStr += gender;

                queryStr += "\')";

                if (!query.exec(queryStr))
                {
                    printf("get error \n");
                }

            }

        }
        fileCsv.close();

        showMember(MEMBER_NEW);

    }

}

void MainWindow::addNewMember(QString gender,QString grade,QString name,QString active)
{

    // issignin integer, islate integer, latecnt integer, attendcnt integer, uniformcnt integer, badgecnt integer
    QSqlQuery query(m_database);

    QString queryStr;
    if(rbTnt->isChecked()){
        queryStr = QString("INSERT INTO OldMembs (name , grade , gender, color, active, issignin , islate , sundaycnt, latecnt , attendcnt , uniformcnt , badgecnt  ) VALUES (\'");
    }
    else if(rbSpark->isChecked() ){
        queryStr = QString("INSERT INTO SparkOldMembs (name , grade , gender, color, active, issignin , islate , sundaycnt , latecnt , attendcnt , uniformcnt , badgecnt  ) VALUES (\'");

    }
    else if(rbCowk->isChecked() ){
        queryStr = QString("INSERT INTO CowkOldMembs (name , grade , gender, color, active, issignin , islate , sundaycnt , latecnt , attendcnt , uniformcnt , badgecnt  ) VALUES (\'");

    }
    else{
        queryStr = QString("INSERT INTO CubiOldMembs (name , grade , gender, color, active, issignin , islate  , sundaycnt, latecnt , attendcnt , uniformcnt , badgecnt  ) VALUES (\'");

    }

    queryStr += name;

    queryStr += "\', \'";
    queryStr += grade;

    queryStr += "\', \'";
    queryStr += gender;

    queryStr += "\', \'";
    queryStr += "Red";

    queryStr += "\', \'";
    queryStr += active;

    queryStr += "\', \'";
    queryStr += QString("0"); //issignin

    queryStr += "\', \'";
    queryStr += QString("0"); //islate

    queryStr += "\', \'";
    queryStr += QString("0"); //latecnt

    queryStr += "\', \'";
    queryStr += QString("0"); //latecnt

    queryStr += "\', \'";
    queryStr += QString("0"); //attendcnt

    queryStr += "\', \'";
    queryStr += QString("0"); //uniformcnt

    queryStr += "\', \'";
    queryStr += QString("0"); //badgecnt

    queryStr += "\')";

    if (!query.exec(queryStr))
    {
        printf("INSERT error:%s \n", qPrintable(queryStr));
    }

}

void MainWindow::addOnSiteMember(int group, QString gender,QString grade,QString name,QString active, QString color, QString sunday)
{
    int scnt = 0;
    if( sunday.contains("yes")){
        scnt = 1;

    }

    // issignin integer, islate integer, latecnt integer, attendcnt integer, uniformcnt integer, badgecnt integer
    QSqlQuery query(m_database);

    QString queryStr;
    if(group == GROUP_TNT){
        queryStr = QString("INSERT INTO OldMembs (name , grade , gender, color, active, issignin , islate , sundaycnt, latecnt , attendcnt , uniformcnt , badgecnt  ) VALUES (\'");
    }
    else if(group == GROUP_SPARK ){
        queryStr = QString("INSERT INTO SparkOldMembs (name , grade , gender, color, active, issignin , islate , sundaycnt , latecnt , attendcnt , uniformcnt , badgecnt  ) VALUES (\'");

    }
    else if(group == GROUP_COWK ){
        queryStr = QString("INSERT INTO CowkOldMembs (name , grade , gender, color, active, issignin , islate , sundaycnt , latecnt , attendcnt , uniformcnt , badgecnt  ) VALUES (\'");

    }
    else{
        queryStr = QString("INSERT INTO CubiOldMembs (name , grade , gender, color, active, issignin , islate  , sundaycnt, latecnt , attendcnt , uniformcnt , badgecnt  ) VALUES (\'");

    }

    queryStr += name;

    queryStr += "\', \'";
    queryStr += grade;

    queryStr += "\', \'";
    queryStr += gender;

    queryStr += "\', \'";
    queryStr += color;

    queryStr += "\', \'";
    queryStr += active;

    queryStr += "\', \'";
    queryStr += QString("1"); //issignin

    queryStr += "\', \'";
    queryStr += QString("0"); //islate

    queryStr += "\', \'";
    queryStr += QString("%1").arg(scnt); //sundaycnt

    queryStr += "\', \'";
    queryStr += QString("0"); //latecnt

    queryStr += "\', \'";
    queryStr += QString("1"); //attendcnt

    queryStr += "\', \'";
    queryStr += QString("0"); //uniformcnt

    queryStr += "\', \'";
    queryStr += QString("0"); //badgecnt

    queryStr += "\')";

    if (!query.exec(queryStr))
    {
        printf("INSERT error:%s \n", qPrintable(queryStr));
    }

}


void MainWindow::deactMember(QString name, QString grade, QString gender )
{

    QSqlQuery query(m_database);

    QString queryStr;

     if(rbTnt->isChecked()){
         queryStr = QString("UPDATE OldMembs SET  active = 'yes' ");
         queryStr += QString(" where name = '%1' and grade = '%2' and gender = '%3' ").arg(name).arg(grade).arg(gender);
    }
    else if(rbSpark->isChecked()){
         queryStr = QString("UPDATE SparkOldMembs SET  active = 'yes' ");
         queryStr += QString(" where name = '%1' and grade = '%2' and gender = '%3' ").arg(name).arg(grade).arg(gender);

    }
    else if(rbCowk->isChecked()){
         queryStr = QString("UPDATE CowkOldMembs SET  active = 'yes' ");
         queryStr += QString(" where name = '%1' and grade = '%2' and gender = '%3' ").arg(name).arg(grade).arg(gender);

    }
    else{
         queryStr = QString("UPDATE CubiOldMembs SET  active = 'yes' ");
         queryStr += QString(" where name = '%1' and grade = '%2' and gender = '%3' ").arg(name).arg(grade).arg(gender);

    }

     if (!query.exec(queryStr))
     {
         qDebug() << "query failed:" << queryStr;
     }
}

void MainWindow::on_btnRemote_clicked()
{

    showMember(MEMBER_OLD);

}

void MainWindow::on_btnUpdateOld_clicked()  //deactive the old not in file
{

    QString text = QString(tr("Deactivate Old Member on the Remote DB. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    printf("update remote db \n");

    int cnt = tbwRemove->rowCount();

    QTableWidgetItem * nameItem;
    QTableWidgetItem * gradeItem;
    QTableWidgetItem * genderItem;

    QString name;
    QString grade;
    QString gender;


    for( int i = 0; i< cnt ; i ++ ){
        nameItem = tbwRemove->item ( i, 0 ) ;
        gradeItem = tbwRemove->item ( i, 1 ) ;
        genderItem = tbwRemove->item ( i, 2 ) ;

        name = nameItem->text();
        grade = gradeItem->text();
        gender = genderItem->text();

        //printf("%s, %s, %s \n", qPrintable(name), qPrintable(grade), qPrintable(gender) );

        deactMember(name, grade, gender);

    }

    showMember(MEMBER_OLD);

}



void MainWindow::on_btnUpdateNew_clicked()// add new from file to DB
{

    QString text = QString(tr("Add New Member to the Remote DB. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    printf("update  db with new member \n");
    int cnt = tbwAdded->rowCount();

    QTableWidgetItem * nameItem;
    QTableWidgetItem * gradeItem;
    QTableWidgetItem * genderItem;

    QString name;
    QString grade;
    QString gender;


    for( int i = 0; i< cnt ; i ++ ){
        nameItem = tbwAdded->item ( i, 0 ) ;
        gradeItem = tbwAdded->item ( i, 1 ) ;
        genderItem = tbwAdded->item ( i, 2 ) ;

        name = nameItem->text();
        grade = gradeItem->text();
        gender = genderItem->text();

        addNewMember(gender,grade,name,"yes"  );

        //printf("%s, %s, %s \n", qPrintable(name), qPrintable(grade), qPrintable(gender) );

    }

    showMember(MEMBER_OLD);


}


void MainWindow::on_btnDiff_clicked()
{

    printf("found diff , m_numNewMemb = %d \n", m_numNewMemb);

    foundNewMemb();
    foundRemoveMemb();

}

 void MainWindow::deleteOneGrade(int group)
 {
     QSqlQuery query(m_database);

     QString queryStr ;

     if(group == GROUP_TNT){
         queryStr= QString("DELETE FROM OldMembs where grade='6th'");

     }
     else if(group == GROUP_SPARK){
         queryStr= QString("DELETE FROM SparkOldMembs where grade='2nd'");
     }
     else if(group == GROUP_COWK){
          queryStr= QString("DELETE FROM CowkOldMembs");
     }
     else if(group == GROUP_CUBI){
          queryStr= QString("DELETE FROM CubiOldMembs");

       // QMessageBox::information(this, tr("Error"), tr("TNT or Sparks"));
     }

     if (!query.exec(queryStr))
     {
         printf("delete failed:%s \n", qPrintable(queryStr));
     }


 }



void MainWindow::upgradeOneGrade(QString grade)
{

    QSqlQuery query(m_database);

    QString queryStr ;

    QString newGrade= "6th";
    QString db = "OldMembs";

    QList<int> idList;

    if( grade.contains("Kin")){
        newGrade = "1st";
    }
    else if( grade.contains("1st")){
        newGrade = "2nd";
    }
    else if( grade.contains("3rd")){
        newGrade = "4th";
    }
    else if( grade.contains("4th")){
        newGrade = "5th";
    }
    else if( grade.contains("5th")){
        newGrade = "6th";
    }


    if(rbTnt->isChecked()){
        db = "OldMembs";
    }
    else if(rbSpark->isChecked()){
        db = "SparkOldMembs";
    }
    else{
       QMessageBox::information(this, tr("Error"), tr("TNT or Sparks"));
    }

    queryStr= QString("SELECT id FROM %1 where grade ='%2'").arg(db).arg(grade);

    if (!query.exec(queryStr))
    {
        printf("delete failed:%s \n", qPrintable(queryStr));
        return ;
    }

    while( query.next() )
    {
        int id = query.value(0).toInt();
        idList.append(id);
    }

    int len = idList.size();
    for(int i = 0; i< len; i++){

        queryStr = QString("UPDATE %1 SET  grade = '%2' ").arg(db).arg(newGrade);
        queryStr += QString(" where id = '%1' ").arg(idList[i]);

        if (!query.exec(queryStr))
        {
            //printf("get error \n");

            qDebug() << "query failed:" << queryStr;

        }
    }

}
 void MainWindow::on_btnNewGrade_clicked()
 {
     if(rbTnt->isChecked()){
         //delete 6th
         deleteOneGrade(GROUP_TNT);

         upgradeOneGrade("5th");

         upgradeOneGrade("4th");
         upgradeOneGrade("3rd");

     }
     else if(rbSpark->isChecked()){
         deleteOneGrade(GROUP_SPARK);
          upgradeOneGrade("1st");
         upgradeOneGrade("Kin");


     }
     else if( rbCowk->isChecked()){
         deleteOneGrade(GROUP_COWK);
     }
     else{
         deleteOneGrade(GROUP_CUBI);
        //QMessageBox::information(this, tr("Error"), tr("Select TNT or Sparks"));
     }

 }

void MainWindow::on_btnCleanInactive_clicked()
{
    QSqlQuery query(m_database);

    QString queryStr ;

    if(rbTnt->isChecked()){
        queryStr= QString("DELETE FROM OldMembs where active = 'no' ");
    }
    else if(rbSpark->isChecked()){
        queryStr= QString("DELETE FROM SparkOldMembs  where active = 'no' ");
    }
    else  if(rbCowk->isChecked()){
         queryStr= QString("DELETE FROM CowkOldMembs  where active = 'no' ");

    }
    else{
          queryStr= QString("DELETE FROM CubiOldMembs  where active = 'no' ");
    }
    if (!query.exec(queryStr))
    {
        printf("delete failed:%s \n", qPrintable(queryStr));
    }
}

void MainWindow::on_btnUpdateCowk_clicked()
{

        QString text = QString(tr("Update Cowork to the Remote DB. Continue?"));
        int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if ( rtn == QMessageBox::No )
        {
            return;
        }
        printf("update remote db \n");

        if(! rbCowk->isChecked()) return;

         m_updateStr="cowk;";

        QString name;
        QString grade;
        QString gender;

        int rCnt = m_newTableModel.rowCount();
        printf( "table row count = %d \n", rCnt);
        int cnt = 0;
        for( int i = 0 ; i< rCnt; i++){
            name = m_newTableModel.record(i).field(0).value().toString();
            grade = m_newTableModel.record(i).field(1).value().toString();
            gender = m_newTableModel.record(i).field(2).value().toString();


            addNewMember(gender,grade,name,"yes"  );

            printf("%s, %s, %s \n", qPrintable(name), qPrintable(grade), qPrintable(gender) );
          //  m_updateStr = m_updateStr + name + "," + grade + "," + gender + ";";

        }

}


void MainWindow::doRbChange()
{
    // deleteTable(MEMBER_OLD);
     showMember(MEMBER_NEW);
     showMember(MEMBER_OLD);

     int cnt = tbwAdded->rowCount();

     for( int i = 0; i< cnt ; i ++ ){
         tbwAdded->removeRow ( cnt - 1 - i );
     }

     cnt = tbwRemove->rowCount();
     printf("foundRemoveMemb cnt = %d \n", cnt);

     for( int i = 0; i< cnt ; i ++ ){
         tbwRemove->removeRow ( cnt- i -1 );
     }

}

void MainWindow::on_rbSparkChange(bool s)
{
    if( s == true){
        doRbChange();
        btnUpdateCowk->setDisabled(true);
    }

}

void MainWindow::on_rbCowkChange(bool s)
{
    if( s == true){
       // btnUpdateCowk->setDisabled(false);
        doRbChange();
    }

}


void MainWindow::on_rbCubiChange(bool s)
{
    if( s == true){
        //btnUpdateCowk->setDisabled(false);
        doRbChange();
    }

}

void MainWindow::on_rbTntChange(bool s)
{
    printf("on_rbTntChange :%d \n", s);
    if( s == true){
        doRbChange();
        btnUpdateCowk->setDisabled(true);
    }


}


void MainWindow::on_rbTntOverviewChange(bool s)
{
    if( s == true){
        showMemberOverview();
    }
}

void MainWindow::on_rbSparkOverviewChange(bool s)
{
    if( s == true){
        showMemberOverview();
    }
}

void MainWindow::on_rbCowkOverviewChange(bool s)
{
    if( s == true){
        showMemberOverview();
    }
}

void MainWindow::on_rbCubiOverviewChange(bool s)
{
    if( s == true){
        showMemberOverview();
    }

}

void MainWindow::resetNewSign()
{
    leName->setText("");
    rbBoy->setChecked(true);
    //rbKin->setChecked();
    rbFlight->setChecked(true);
    rbStarZone->setChecked(true);
    sbChptSpark->setValue(0);
    sbChptTnt->setValue(1);
    cbSunday->setChecked(false);
 }

QString MainWindow::getGender()
{
    QString str="";
    if( rbBoy->isChecked()){
        str="Boy";
    }
    else{
        str="Girl";
    }

    return str;
}
QString MainWindow::getGrade()
{
    QString str="";
    if( rbKin->isChecked()){
        str="Kin";
    }
    else  if( rb1st->isChecked()){
        str="1st";
    }
    else  if( rb2nd->isChecked()){
        str="2nd";
    }
    else  if( rb3rd->isChecked()){
        str="3rd";
    }
    else  if( rb4th->isChecked()){
        str="4th";
    }
    else  if( rb5th->isChecked()){
        str="5th";
    }
    else  if( rb6th->isChecked()){
        str="6th";
    }
    return str;
}

QString MainWindow::getSunday()
{
    QString str="";
    if( cbSunday->isChecked())
    {
        str="yes";
    }
    else{

        str = "no";
    }
    return str;
}

QString MainWindow::getColor()
{
    QString str="";
    if( rbRed->isChecked()){
        str="Red";
    }
    else if(rbGreen->isChecked()){
        str="Green";
    }
    else if(rbBlue->isChecked()){
        str="Blue";
    }
    else {
        str="Yellow";
    }
    return str;
}
QString MainWindow::getBook(QString grade)
{
    QString str="";
    QString b="0";
    QString c="0";
    if( (grade.contains("Kin")) || (grade.contains("1st")) || (grade=="2nd"))
    {
        if(rbFlight->isChecked()){
            b="0";
        }
        else if(rbGlider->isChecked()){
            b="1";
        }
        else if(rbRunner->isChecked()){
            b="3";
        }
        else if(rbStormer->isChecked()){
            b="5";
        }

        c= sbChptSpark->text();
        str = b + "_" + c + "_1";
    }
    else if((grade == "3rd") || (grade == "4th")){
        if( rbStarZone->isChecked()){
            b="1";
        }
        else if( rbBook1->isChecked()){
            b="2";
        }
        else if( rbBook2->isChecked()){
            b="3";
        }

        c=  sbChptTnt->text();
        str = b + "_" + c + "_1";

    }
    else{
        if( rbStarZone->isChecked()){
            b="4";
        }
        else if( rbBook1->isChecked()){
            b="5";
        }
        else if( rbBook2->isChecked()){
            b="6";
        }

        c=  sbChptTnt->text();
        str = b + "_" + c + "_1";
    }

    return str;
}

void MainWindow::addSigninTable(QString name,QString grade,QString gender,QString color, QString sunday, QString book,QString update)
{
    if( (grade=="Kin") || (grade=="1st") || (grade=="2nd"))
    {
        tbwNewSigninSpark->setUpdatesEnabled(false);

        int rowCnt = tbwNewSigninSpark->rowCount();
        tbwNewSigninSpark->setRowCount(rowCnt + 1);

        tbwNewSigninSpark->setItem(rowCnt, 0, new QTableWidgetItem(name));
        tbwNewSigninSpark->setItem(rowCnt, 1, new QTableWidgetItem(grade));
        tbwNewSigninSpark->setItem(rowCnt, 2, new QTableWidgetItem(gender));
        tbwNewSigninSpark->setItem(rowCnt, 3, new QTableWidgetItem(color));
        tbwNewSigninSpark->setItem(rowCnt, 4, new QTableWidgetItem(sunday));
        tbwNewSigninSpark->setItem(rowCnt, 5, new QTableWidgetItem(book));
        tbwNewSigninSpark->setItem(rowCnt, 6, new QTableWidgetItem(update));

        tbwNewSigninSpark->setUpdatesEnabled(true);

    }
    else{

        tbwNewSignin->setUpdatesEnabled(false);

        int rowCnt = tbwNewSignin->rowCount();
        tbwNewSignin->setRowCount(rowCnt + 1);

        tbwNewSignin->setItem(rowCnt, 0, new QTableWidgetItem(name));
        tbwNewSignin->setItem(rowCnt, 1, new QTableWidgetItem(grade));
        tbwNewSignin->setItem(rowCnt, 2, new QTableWidgetItem(gender));
        tbwNewSignin->setItem(rowCnt, 3, new QTableWidgetItem(color));
        tbwNewSignin->setItem(rowCnt, 4, new QTableWidgetItem(sunday));
        tbwNewSignin->setItem(rowCnt, 5, new QTableWidgetItem(book));
        tbwNewSignin->setItem(rowCnt, 6, new QTableWidgetItem(update));

        tbwNewSignin->setUpdatesEnabled(true);
    }
}

void MainWindow::on_btnAddSignin_clicked()
{
    if( leName->text().isEmpty()) return;

    QString name = leName->text().trimmed();
    QString grade = getGrade();
    QString gender = getGender();
    QString color = getColor();
    QString sunday = getSunday();
     QString book = getBook(grade);
    QString updb ="no";

    addSigninTable(name,grade,gender,color,sunday,book,updb);

    QSqlQuery query(m_database);
    QString queryStr;

    queryStr = QString("INSERT INTO NewSignin (name , grade , gender,color, sunday, book, updb ) VALUES (\'");

    queryStr += name;

    queryStr += "\', \'";
    queryStr += grade;

    queryStr += "\', \'";
    queryStr += gender;

    queryStr += "\', \'";
    queryStr += color;

    queryStr += "\', \'";
    queryStr += sunday;

    queryStr += "\', \'";
    queryStr += book;

    queryStr += "\', \'";
    queryStr += updb;

    queryStr += "\')";

    if (!query.exec(queryStr))
    {
        printf("get error \n");

        qDebug() << "query failed:" << queryStr;

       // qDebug() <<
    }

    QString db="SininMembs";

    if( (grade=="Kin") || (grade=="1st") || (grade=="2nd"))
    {
        db= "SparkSigninMembs";
    }


    queryStr = QString("INSERT INTO %1 (name , grade , gender, color, signin ) VALUES (\'").arg(db);

    queryStr += name;

    queryStr += "\', \'";
    queryStr += grade;

    queryStr += "\', \'";
    queryStr += gender;

    queryStr += "\', \'";
    queryStr += color;

    queryStr += "\', \'";
    queryStr += "yes";

    queryStr += "\')";

    if (!query.exec(queryStr))
    {
        printf("get error:%s \n", qPrintable(queryStr));
    }

    resetNewSign();
}

void MainWindow::retrieveSignin(bool isAll)
{
    QString name = "";
    QString grade = "";
    QString gender = "";
    QString color = "";
    QString sunday = "";
    QString book = "";
    QString updb= "";

    QSqlQuery query(m_database);
    QString sqlStr;

    if(isAll){
        sqlStr = QString("select * from NewSignin");
    }
    else{
        sqlStr = QString("select * from NewSignin where updb='no'");
    }

    if(!query.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));

        qDebug() << "chkQuery failed";
    }


    if (query.isActive()) {

        while ( query.next() ) {

            name = query.value(0).toString();
            grade = query.value(1).toString();
            gender = query.value(2).toString();
            color = query.value(3).toString();
            sunday = query.value(4).toString();
             book = query.value(5).toString();
            updb = query.value(6).toString();

            qDebug() << "get one singin";
            addSigninTable(name,grade,gender,color,sunday, book,updb);

        }
    }


}

void MainWindow::on_btnAllSignin_clicked()
{
    cleanSigninTable();
    qDebug() << "on_btnAllSignin_clicked";
    retrieveSignin(true);

}

void MainWindow::on_btnHideUpDB_clicked()
{
    cleanSigninTable();

    retrieveSignin(false);

}

void MainWindow::delSigninDB(QString name, QString grade, QString gender)
{
    QSqlQuery query(m_database);
    QString queryStr;

    queryStr = QString("DELETE FROM NewSignin where ");

    queryStr += QString("name = '%1' ").arg(name);

    queryStr += QString("and grade='%1' ").arg(grade);

    queryStr += QString("and gender='%1' ").arg(gender);

    //queryStr += QString("and color='%1' ").arg(color);

    //queryStr += QString("and book='%1' ").arg(book);

    //queryStr += QString("and updb='%1' ").arg(updb);

     qDebug() <<  queryStr;

    if (!query.exec(queryStr))
    {
        printf("get error \n");

        qDebug() << "query failed:" << queryStr;

    }


}


void MainWindow::updateNewSignin(QString name, QString grade, QString gender)
{
    QSqlQuery query(m_database);
    QString queryStr;


    queryStr = QString("UPDATE  NewSignin SET updb = 'yes' where ");

    queryStr += QString("name = '%1' ").arg(name);

    queryStr += QString("and grade='%1' ").arg(grade);

    queryStr += QString("and gender='%1' ").arg(gender);

    //queryStr += QString("and color='%1' ").arg(color);

    //queryStr += QString("and book='%1' ").arg(book);


    if (!query.exec(queryStr))
    {
        printf("get error \n");

        qDebug() << "query failed:" << queryStr;

    }

}


void MainWindow::on_btnDelSignin_clicked()
{
    QList<QTableWidgetItem*> items = tbwNewSignin->selectedItems();
    if ( items.count() == 0 ) {

        return;
    }


    printf( "selecte item cnt :%d \n", items.count());
    int i = items[0]->row();


    QTableWidgetItem * nameItem = tbwNewSignin->item ( i, 0 ) ;
    QTableWidgetItem * gradeItem = tbwNewSignin->item ( i, 1 ) ;
    QTableWidgetItem * genderItem = tbwNewSignin->item ( i, 2 ) ;
    //QTableWidgetItem * colorItem = tbwNewSignin->item ( i, 3 ) ;
    //QTableWidgetItem * bookItem = tbwNewSignin->item ( i, 5 ) ;
    QTableWidgetItem * updbItem = tbwNewSignin->item ( i, 6 ) ;

    QString name = nameItem->text();
    QString grade = gradeItem->text();
    QString gender = genderItem->text();
    //QString color = colorItem->text();
    //QString book = bookItem->text();
    QString updb = updbItem->text();

    if( updb == "yes")  return;

    QString text = QString(tr("Remove the New Signin from Tnt. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }

    tbwNewSignin->removeRow (  i );

    delSigninDB( name,  grade,  gender);
}

void MainWindow::on_btnDelSigninSpark_clicked()
{
    QList<QTableWidgetItem*> items = tbwNewSigninSpark->selectedItems();
    if ( items.count() == 0 ) {

        return;
    }


    printf( "selecte item cnt :%d \n", items.count());
    int i = items[0]->row();

    QTableWidgetItem * nameItem = tbwNewSigninSpark->item ( i, 0 ) ;
    QTableWidgetItem * gradeItem = tbwNewSigninSpark->item ( i, 1 ) ;
    QTableWidgetItem * genderItem = tbwNewSigninSpark->item ( i, 2 ) ;
    //QTableWidgetItem * colorItem = tbwNewSigninSpark->item ( i, 3 ) ;
    //QTableWidgetItem * bookItem = tbwNewSigninSpark->item ( i, 5 ) ;
    QTableWidgetItem * updbItem = tbwNewSigninSpark->item ( i, 6 ) ;

    QString name = nameItem->text();
    QString grade = gradeItem->text();
    QString gender = genderItem->text();
    //QString color = colorItem->text();
    //QString book = bookItem->text();
    QString updb = updbItem->text();

    if( updb == "yes")  return;

    QString text = QString(tr("Remove the New Signin from Sparks. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    tbwNewSigninSpark->removeRow (  i );

    delSigninDB( name,  grade,  gender);
}

void MainWindow::on_btnUpdateNewSignin_clicked() //on site signin member
{

        QString text = QString(tr("Add OnSite Signin Member to  DB. Continue?"));
        int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if ( rtn == QMessageBox::No )
        {
            return;
        }
        printf("update remote db \n");


        QTableWidgetItem * nameItem;
        QTableWidgetItem * gradeItem;
        QTableWidgetItem * genderItem;
        QTableWidgetItem * colorItem;
        QTableWidgetItem * sundayItem;
        QTableWidgetItem * bookItem;
        QTableWidgetItem * updbItem;

        QString name;
        QString grade;
        QString gender;
        QString color;
        QString sunday;
        QString book;
        QString updb;

        QString sparkStr="";
        m_updateStr="";

        int cnt = tbwNewSignin->rowCount();

        for( int i = 0; i< cnt ; i ++ ){

            nameItem = tbwNewSignin->item ( i, 0 ) ;
            gradeItem = tbwNewSignin->item ( i, 1 ) ;
            genderItem = tbwNewSignin->item ( i, 2 ) ;
            colorItem = tbwNewSignin->item ( i, 3 ) ;
            sundayItem = tbwNewSignin->item ( i, 4 ) ;
            bookItem = tbwNewSignin->item ( i, 5 ) ;
            updbItem = tbwNewSignin->item ( i, 6 ) ;

            name = nameItem->text();
            grade = gradeItem->text();
            gender = genderItem->text();
            color = colorItem->text();
            sunday = sundayItem->text();
            book = bookItem->text();
            updb = updbItem->text();

           // if( updb == "yes") continue;

            tbwNewSignin->setItem(i, 6, new QTableWidgetItem("yes"));
            updateNewSignin( name,  grade,  gender);
            addOnSiteMember(GROUP_TNT, gender,grade,name,"yes" , color, sunday );

           // m_updateStr = m_updateStr + name + "," + grade + "," + gender +  "," + color + "," + sunday + "," + book + ";";

        }

        cnt = tbwNewSigninSpark->rowCount();

        for( int i = 0; i< cnt ; i ++ ){

            nameItem = tbwNewSigninSpark->item ( i, 0 ) ;
            gradeItem = tbwNewSigninSpark->item ( i, 1 ) ;
            genderItem = tbwNewSigninSpark->item ( i, 2 ) ;
            colorItem = tbwNewSigninSpark->item ( i, 3 ) ;
            sundayItem = tbwNewSigninSpark->item ( i, 4 ) ;
             bookItem = tbwNewSigninSpark->item ( i, 5 ) ;
            updbItem = tbwNewSigninSpark->item ( i, 6 ) ;

            name = nameItem->text();
            grade = gradeItem->text();
            gender = genderItem->text();
            color = colorItem->text();
            sunday = sundayItem->text();
             book = bookItem->text();
            updb = updbItem->text();
           // if( updb == "yes") continue;

            tbwNewSigninSpark->setItem(i, 6, new QTableWidgetItem("yes"));
            updateNewSignin( name,  grade,  gender);
            addOnSiteMember(GROUP_SPARK, gender,grade,name,"yes", color, sunday  );

           // sparkStr = sparkStr + name + "," + grade + "," + gender +  "," + color + ","  + sunday + "," + book + ";";

        }

        m_httpCnt = 0;
        /*
        if(! m_updateStr.isEmpty()){
            m_updateStr = m_updateStr + "End_";
            httpPostNewMemb(4);
        }

        if(!sparkStr.isEmpty() ){

            sparkStr = sparkStr + "End_";
            m_updateStr = sparkStr;
            httpPostNewMemb(5);

        }*/

        //change database to yes
        //cleanSigninTable();

}

void MainWindow::cleanSigninTable()
{
    int cnt = tbwNewSignin->rowCount();
    for( int i = 0; i< cnt ; i ++ ){
        tbwNewSignin->removeRow ( cnt - 1 - i );
    }

    cnt = tbwNewSigninSpark->rowCount();
    for( int i = 0; i< cnt ; i ++ ){
        tbwNewSigninSpark->removeRow ( cnt - 1 - i );
    }
}

void MainWindow::mySleep (int milliseconds)
{
        QWaitCondition sleep;
        QMutex *mt = new QMutex;

        if (milliseconds == 0) {
            delete mt;
            return;
        }
        mt->lock ();
        sleep.wait (mt, milliseconds);
        mt->unlock ();
}


void MainWindow::on_btnStartSign_clicked()
{

    printf("on_btnStartSign_clicked\n");
    SigninDialog dlg(this);

    dlg.setScanMode(m_scanSignin);


    int rtn = dlg.exec();
    printf("return from singin \n");


}


void MainWindow::updateLocalSignin(int group, QString name, QString sunday, QString uniform, QString late, QString badge)
{

    //printf("updateLocalSignin:%d \n", group);
    QSqlQuery query(m_database);
    QString queryStr;

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


    queryStr = QString("UPDATE %1 SET signin='yes', sunday = '%2', late= '%3', uniform='%4', badge='%5'").arg(db).arg(sunday).arg(late).arg(uniform).arg(badge);

    queryStr += QString(" where name = '%1' ").arg(name);


    if (!query.exec(queryStr))
    {
        printf("get error \n");

        qDebug() << "query failed:" << queryStr;

    }

    m_signNum++;
    leNumSignin->setText(QString("%1").arg(m_signNum));
    m_loadSignState = 3;
    btnLoadSignin->setDisabled(true);


    queryStr = QString("UPDATE SigninStatus  SET loadstatus ='%1', num2bUpd  = '%2'").arg(m_loadSignState).arg(m_signNum);


    query.exec(queryStr);


}

void  MainWindow::slotGotSignin(QString name, QString color, int group, bool isSunday, bool isUniform, bool isLate, bool isBadge)
{
    printf("slotGotSignin: g=%d \n", group);
    QString sunday="no";
    QString uniform="no";
    QString late="no";
    QString badge="no";

    if(isSunday) sunday="yes";
    if(isLate)   late="yes";
    if(isUniform) uniform="yes";
    if(isBadge) badge="yes";


    int rowCnt = 0;
    switch(group){
    case GROUP_TNT:


        rowCnt = tbwSignInTnt->rowCount();
        tbwSignInTnt->setRowCount(rowCnt + 1);

        tbwSignInTnt->setItem(rowCnt, 0, new  QTableWidgetItem(name));
        tbwSignInTnt->setItem(rowCnt, 1, new  QTableWidgetItem(color));
        tbwSignInTnt->setItem(rowCnt, 2, new  QTableWidgetItem(late));
        tbwSignInTnt->setItem(rowCnt, 3, new  QTableWidgetItem(uniform));
        tbwSignInTnt->setItem(rowCnt, 4, new  QTableWidgetItem(badge));
        tbwSignInTnt->setItem(rowCnt, 5, new  QTableWidgetItem(sunday));

        break;
    case GROUP_SPARK:
          rowCnt = tbwSignInSpark->rowCount();
        tbwSignInSpark->setRowCount(rowCnt + 1);

        tbwSignInSpark->setItem(rowCnt, 0, new  QTableWidgetItem(name));
        tbwSignInSpark->setItem(rowCnt, 1, new  QTableWidgetItem(color));
        tbwSignInSpark->setItem(rowCnt, 2, new  QTableWidgetItem(late));
        tbwSignInSpark->setItem(rowCnt, 3, new  QTableWidgetItem(uniform));
        tbwSignInSpark->setItem(rowCnt, 4, new  QTableWidgetItem(badge));
        tbwSignInSpark->setItem(rowCnt, 5, new  QTableWidgetItem(sunday));

        break;

    case GROUP_COWK:

        sunday=QTime::currentTime().toString("hh:mm ap");

        rowCnt = tbwSignInCowk->rowCount();

        tbwSignInCowk->setRowCount(rowCnt + 1);

        tbwSignInCowk->setItem(rowCnt, 0, new  QTableWidgetItem(name));
        tbwSignInCowk->setItem(rowCnt, 1, new  QTableWidgetItem(color));
        tbwSignInCowk->setItem(rowCnt, 2, new  QTableWidgetItem(late));
        tbwSignInCowk->setItem(rowCnt, 3, new  QTableWidgetItem(uniform));
        tbwSignInCowk->setItem(rowCnt, 4, new  QTableWidgetItem(badge));
        tbwSignInCowk->setItem(rowCnt, 5, new  QTableWidgetItem(sunday));

        break;

    case GROUP_CUBI:

        rowCnt = tbwSignInCubi->rowCount();
        tbwSignInCubi->setRowCount(rowCnt + 1);

        tbwSignInCubi->setItem(rowCnt, 0, new  QTableWidgetItem(name));
        tbwSignInCubi->setItem(rowCnt, 1, new  QTableWidgetItem(color));
        tbwSignInCubi->setItem(rowCnt, 2, new  QTableWidgetItem(late));
        tbwSignInCubi->setItem(rowCnt, 3, new  QTableWidgetItem(uniform));
        tbwSignInCubi->setItem(rowCnt, 4, new  QTableWidgetItem(badge));
        tbwSignInCubi->setItem(rowCnt, 5, new  QTableWidgetItem(sunday));

        break;

    }

    updateLocalSignin(group, name, sunday, uniform, late, badge);


}



void MainWindow::on_btnSigninUpd_clicked()
{
    m_httpCnt = 0;


        QString name;
        QString sunday;
        QString late;
        QString uniform;
        QString badge;

        QSqlQuery chkQuery(m_database);
        QString sqlStr;

        m_updateStr="tnt;";

        sqlStr = QString("SELECT name, sunday, late, uniform, badge FROM SininMembs where signin = 'yes'");

        if(!chkQuery.exec(sqlStr))
        {
            printf("chkQuery failed:%s \n", qPrintable(sqlStr));
            return;
        }

        while( chkQuery.next() )
        {

            name =    chkQuery.value(0).toString();
            sunday =  chkQuery.value(1).toString();
            late =    chkQuery.value(2).toString();
            uniform = chkQuery.value(3).toString();
            badge =   chkQuery.value(4).toString();

           // m_updateStr = m_updateStr + name + "," + sunday + "," + late + "," + uniform + "," + badge + ";";

        }
        //m_updateStr = m_updateStr + "End_";

        httpPostSignInMemb(GROUP_TNT, name ,  sunday ,late ,uniform , badge);

        printf("tnt:%s \n", qPrintable(m_updateStr));


        m_updateStr="spark;";

        sqlStr = QString("SELECT name, sunday, late, uniform, badge FROM SparkSigninMembs where signin = 'yes'");

        if(!chkQuery.exec(sqlStr))
        {
            printf("chkQuery failed:%s \n", qPrintable(sqlStr));
            return;
        }

        while( chkQuery.next() )
        {

            name =    chkQuery.value(0).toString();
            sunday =  chkQuery.value(1).toString();
            late =    chkQuery.value(2).toString();
            uniform = chkQuery.value(3).toString();
            badge =   chkQuery.value(4).toString();

           // m_updateStr = m_updateStr + name + "," + sunday + "," + late + "," + uniform + "," + badge + ";";

        }
       // m_updateStr = m_updateStr + "End_";
       httpPostSignInMemb(GROUP_SPARK, name ,  sunday ,late ,uniform , badge);

        printf("spark:%s \n", qPrintable(m_updateStr));


        m_updateStr="cowk;";

        sqlStr = QString("SELECT name, sunday, late, uniform, badge FROM CowkSigninMembs where signin = 'yes'");

        if(!chkQuery.exec(sqlStr))
        {
            printf("chkQuery failed:%s \n", qPrintable(sqlStr));
            return;
        }

        while( chkQuery.next() )
        {

            name =    chkQuery.value(0).toString();
            sunday =  chkQuery.value(1).toString();
            late =    chkQuery.value(2).toString();
            uniform = chkQuery.value(3).toString();
            badge =   chkQuery.value(4).toString();

           // m_updateStr = m_updateStr + name + "," + sunday + "," + late + "," + uniform + "," + badge + ";";

        }
       // m_updateStr = m_updateStr + "End_";
        httpPostSignInMemb(GROUP_COWK, name ,  sunday ,late ,uniform , badge);


        m_updateStr="cubi;";

        sqlStr = QString("SELECT name, sunday, late, uniform, badge FROM CubiSigninMembs where signin = 'yes'");

        if(!chkQuery.exec(sqlStr))
        {
            printf("chkQuery failed:%s \n", qPrintable(sqlStr));
            return;
        }

        while( chkQuery.next() )
        {

            name =    chkQuery.value(0).toString();
            sunday =  chkQuery.value(1).toString();
            late =    chkQuery.value(2).toString();
            uniform = chkQuery.value(3).toString();
            badge =   chkQuery.value(4).toString();

           // m_updateStr = m_updateStr + name + "," + sunday + "," + late + "," + uniform + "," + badge + ";";

        }
        //m_updateStr = m_updateStr + "End_";
        httpPostSignInMemb(GROUP_CUBI, name ,  sunday ,late ,uniform , badge);

        printf("cowk:%s \n", qPrintable(m_updateStr));


}


void MainWindow::genLocalSignin(int group, QString name, QString grade, QString gender, QString color, QString signin)
{

   // printf("genLocalSignin group:%d \n", group);
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


    QSqlQuery query(m_database);
    QString queryStr;

    //printf("genLocalSignin: %s, %s, %s, %s \n", qPrintable(name), qPrintable(grade), qPrintable(gender), qPrintable(signin));

    queryStr = QString("INSERT INTO %1 (name , grade , gender, color, signin ) VALUES (\'").arg(db);

    queryStr += name;

    queryStr += "\', \'";
    queryStr += grade;

    queryStr += "\', \'";
    queryStr += gender;

    queryStr += "\', \'";
    queryStr += color;

    queryStr += "\', \'";
    queryStr += signin;

    queryStr += "\')";

    if (!query.exec(queryStr))
    {
        printf("get error:%s \n", qPrintable(queryStr));
    }

    //printf("loadSignin:%s \n", qPrintable(queryStr));

}

void MainWindow::delSignin()
{
    printf("delSignin \n");

    QSqlQuery query(m_database);

    QString queryStr ;

    queryStr= QString("DELETE FROM SininMembs");
    if (!query.exec(queryStr))
    {
        printf("delete failed:%s \n", qPrintable(queryStr));
    }

    queryStr= QString("DELETE FROM SparkSigninMembs");
    if (!query.exec(queryStr))
    {
        printf("delete failed:%s \n", qPrintable(queryStr));
    }

    queryStr= QString("DELETE FROM CowkSigninMembs");
    if (!query.exec(queryStr))
    {
        printf("delete failed:%s \n", qPrintable(queryStr));
    }

    queryStr= QString("DELETE FROM CubiSigninMembs");
    if (!query.exec(queryStr))
    {
        printf("delete failed:%s \n", qPrintable(queryStr));
    }

}

void MainWindow::httpPostSignInMemb(int group , QString name , QString  sunday , QString late , QString uniform , QString badge)
{

    m_httpState = OPCODE_POST_SIGNIN ;
    int sundaycnt, latecnt, uniformcnt , badgecnt, attendcnt;
    int isLate = 0;

    QSqlQuery query(m_database);

    QString queryStr;
    QString db = "OldMembs";

    switch(group){
    case GROUP_TNT :
         db = "OldMembs";
        break;
    case GROUP_SPARK :
         db = "SparkOldMembs";
        break;
    case GROUP_COWK :
         db = "CowkOldMembs";
        break;
    case GROUP_CUBI :
         db = "CubiOldMembs";
        break;
    }

    queryStr = QString("SELECT sundaycnt, latecnt, uniformcnt , badgecnt, attendcnt from %1 where name = '%2' and issignin = '0' ").arg(db).arg(name);

    bool reply = query.exec(queryStr);

    if(!reply){
      //printf();
    }

    if ( query.next() )
    {

        sundaycnt   =  query.value(0).toInt();
        latecnt     =  query.value(1).toInt();
        uniformcnt  =  query.value(2).toInt();
        badgecnt    =  query.value(3).toInt();
        attendcnt   =  query.value(4).toInt();
     }
    else {
        printf("no record found for: %s \n", qPrintable(name));
    }

    if(sunday.contains("yes")){
        sundaycnt++;

    }
    if(late.contains("yes")){
        latecnt++;
        isLate = 1;

    }
    if(uniform.contains("yes")){
        uniformcnt++;

    }
    if(badge.contains("yes")){
        badgecnt++;

    }
    attendcnt++;

    queryStr = QString("UPDATE %1 SET  issignin ='1' , islate ='%2', sundaycnt = '%3',latecnt ='%4', attendcnt ='%5', uniformcnt ='%6', badgecnt ='%7'")
            .arg(db).arg(isLate).arg(sundaycnt).arg(latecnt).arg(attendcnt).arg(uniformcnt).arg(badgecnt);

    queryStr += QString(" where name = '%1'  ").arg(name);



     if (!query.exec(queryStr))
     {
         qDebug() << "query failed:" << queryStr;
     }


    lbSigninStatus->setText("Done");
    m_signNum = 0;
    leNumSignin->setText(QString("%1").arg(m_signNum));
    m_loadSignState = 2;

   // QSqlQuery query(m_database);
    queryStr = QString("UPDATE SigninStatus  SET loadstatus ='%1', num2bUpd  = '%2'  ").arg(m_loadSignState).arg(m_signNum);

    query.exec(queryStr);

    btnLoadSignin->setDisabled(false);

}

void MainWindow::httpStartGame(int group)
{
    m_httpState = OPCODE_START_GAME ;

    QSqlQuery query(m_database);

    QString queryStr;

    QString db = "OldMembs";

    switch(group){
    case GROUP_TNT :
         db = "OldMembs";
        break;
    case GROUP_SPARK :
         db = "SparkOldMembs";
        break;
    }

    queryStr = QString("UPDATE  %1  SET issignin ='0' , islate ='0' ").arg(db);

    query.exec(queryStr);


    m_loadSignState = 1;
    btnLoadSignin->setDisabled(false);
    lbGameStatus->setText("Done");

}

int MainWindow::getColorNumber(int c)
{
    QSqlQuery query(m_database);

    QString sqlStr;

    QString db = "OldMembs";

    if(rbTntOverview->isChecked()){
        db = "OldMembs";

    }
    else if(rbSparkOverview->isChecked()){
        db = "SparkOldMembs";
    }

    QString color ="Red";
    switch(c){
    case 1:
       color ="Red";
       break;
    case 2:
       color ="Green";
       break;
    case 3:
       color ="Blue";
       break;
    case 4:
       color ="Yellow";
       break;

    }

    sqlStr = QString("SELECT id FROM %1 where color='%2' and active = 'yes'").arg(db).arg(color);

    if(!query.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return 0;
    }

    int len = 0;
    while( query.next() )
    {
        len ++  ;


    }
    return len;
}


void MainWindow::on_btnColorTeams_clicked()
{
    uint qT = QTime::currentTime().msecsSinceStartOfDay();
    qsrand(qT);

    lbTeamRed->setText(QString("Red: 0"));
    lbTeamGreen->setText(QString("Green: 0"));
    lbTeamBlue->setText(QString("Blue: 0"));
    lbTeamYellow->setText(QString("Yellow: 0"));

    mySleep(100);

    int group = GROUP_TNT;
    if(rbTntOverview->isChecked()){

         //printf("colorMembClass for OldMembs:%d \n", qT);
         group = GROUP_TNT;

         colorMembClass(group, "Boy", "6th");
         colorMembClass(group, "Girl", "6th");
         colorMembClass(group, "Boy", "5th");
         colorMembClass(group, "Girl", "5th");
         colorMembClass(group, "Boy", "4th");
         colorMembClass(group, "Girl", "4th");
         colorMembClass(group, "Boy", "3rd");
         colorMembClass(group, "Girl", "3rd");


    }
    else if(rbSparkOverview->isChecked()){

        group = GROUP_SPARK;

       // printf("colorMembClass for SparkOldMembs:%d \n", qT);

        colorMembClass(group, "Boy", "2nd");
        colorMembClass(group, "Girl", "2nd");
        colorMembClass(group, "Boy", "1st");
        colorMembClass(group, "Girl", "1st");
        colorMembClass(group, "Boy", "Kin");
        colorMembClass(group, "Girl", "Kin");

    }

    showMemberOverview();

}

void MainWindow::colorMembClass(int group, QString gender, QString grade)
{

    QSqlQuery query(m_database);

    QString sqlStr;
    QList<int> idList ;

    QString db = "OldMembs";

    switch(group){
    case GROUP_TNT :
         db = "OldMembs";
        break;
    case GROUP_SPARK :
         db = "SparkOldMembs";
        break;
    }

    sqlStr = QString("SELECT id FROM %1 where gender='%2' and grade = '%3' and active = 'yes'").arg(db).arg(gender).arg(grade);

    if(!query.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return;
    }

    int len = 0;
    while( query.next() )
    {
        len ++  ;
        int id = query.value(0).toInt();
        idList.append(id);

    }

    //printf("get number : %d \n", len);

     if(len == 0 ) return;

     QList<int> colorA   ;


       int n = len /4;
       int rem  = len% 4;


       int i;
       for ( i = 0; i< len; i ++ )
       {
          colorA.append(0);
       }

       int j;
       for (j = 1; j <= 4 ; j ++ ){

           for ( i = 0; i < n  ; i ++)
           {
               int rand = qrand();
              int ran = rand % len;
               //printf("[%d:%d]", rand, ran);

              if( colorA[ran] == 0 ) {
                  colorA[ran] = j;
              }
              else{
                int k;

                for (k = 0; k< len ; k++ ){
                  if( colorA[k] != 0 ) continue;
                  colorA[k] = j;
                  break;
                }
              }
           }
       }

       for( i = 0; i< rem; i++){
            //double y = Math.random() * 100;
            //int x = y.intValue();
            int ran = qrand()%len;

            int colr = qrand()%4 +1;

              if( colorA[ran] == 0 ) {
                  colorA[ran] = colr;

              }
              else{
                int k;

                for (k = 0; k< len ; k++ ){
                  if( colorA[k] != 0 ) continue;
                  colorA[k] = colr;
                  break;

                }
              }
       }

      for ( i = 0; i< len; i ++ )
      {
          QString color = "Red";

          if( colorA[i] == 1 ){
              color = "Red";
          }
          else if( colorA[i] == 2){
              color = "Green";

          }
          else if( colorA[i] == 3){
              color = "Blue";

          }
          else if( colorA[i] == 4){
              color = "Yellow";

          }
          //printf("%d:",colorA[i] );

          sqlStr = QString("UPDATE  %1  SET color ='%2' where id = '%3'").arg(db).arg(color).arg(idList.at(i));

          if(!query.exec(sqlStr)){
              printf("Error: %s \n",qPrintable(sqlStr) );
          }
       }
}


void MainWindow::getSigninClass(int group, QString gender, QString grade )
{
    QString name;
    QString color;
    int issignin;

    QSqlQuery query(m_database);

    QString queryStr;

    QString db = "OldMembs";
    switch(group){
    case GROUP_TNT :
         db = "OldMembs";
        break;
    case GROUP_SPARK :
         db = "SparkOldMembs";
        break;
    case GROUP_COWK :
         db = "CowkOldMembs";
        break;
    case GROUP_CUBI :
         db = "CubiOldMembs";
        break;
    }

    queryStr = QString("SELECT name, color, issignin from %1 where  gender = '%2' and grade ='%3' ").arg(db).arg(gender).arg(grade);
    queryStr += QString(" and active  = 'yes'  ");

    bool reply = query.exec(queryStr);

    if(!reply){
      printf("DB ERROR:%s \n", qPrintable(queryStr));

      return;
    }

    while ( query.next() )
    {

        name   =  query.value(0).toString();
        color     =  query.value(1).toString();
        issignin  =  query.value(2).toInt();
        if(issignin == 1){
            m_httpStr = m_httpStr + ";" + name + "," + grade + "," + gender + "," + color + ",yes";
        }
        else{
            m_httpStr = m_httpStr + ";" + name + "," + grade + "," + gender + "," + color + ",no" ;
        }
     }


}

void MainWindow::httpGetSignInMemb(int group)
{


    //QString url;

    switch(group){
    case GROUP_TNT:
        //url = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocSignin";
        m_httpStr = "tnt";
        getSigninClass(group, "Boy", "6th");
        getSigninClass(group, "Girl", "6th");
        getSigninClass(group, "Boy", "5th");
        getSigninClass(group, "Girl", "5th");
        getSigninClass(group, "Boy", "4th");
        getSigninClass(group, "Girl", "4th");
        getSigninClass(group, "Boy", "3rd");
        getSigninClass(group, "Girl", "3rd");

        break;
    case GROUP_SPARK:
        m_httpStr = "spark";

        getSigninClass(group, "Boy", "2nd");
        getSigninClass(group, "Girl", "2nd");
        getSigninClass(group, "Boy", "1st");
        getSigninClass(group, "Girl", "1st");
        getSigninClass(group, "Boy", "Kin");
        getSigninClass(group, "Girl", "Kin");

        break;

    case GROUP_COWK:
        m_httpStr = "cowk";
        //url = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocCowkSignin";
        getSigninClass(group, "Boy", "6th");
        getSigninClass(group, "Girl", "6th");
        getSigninClass(group, "Boy", "5th");
        getSigninClass(group, "Girl", "5th");
        getSigninClass(group, "Boy", "4th");
        getSigninClass(group, "Girl", "4th");
        getSigninClass(group, "Boy", "3rd");
        getSigninClass(group, "Girl", "3rd");
        getSigninClass(group, "Boy", "2nd");
        getSigninClass(group, "Girl", "2nd");
        getSigninClass(group, "Boy", "1st");
        getSigninClass(group, "Girl", "1st");
        getSigninClass(group, "Boy", "Kin");
        getSigninClass(group, "Girl", "Kin");
        getSigninClass(group, "Boy", "Cub");
        getSigninClass(group, "Girl", "Cub");

        break;
    case GROUP_CUBI:
        m_httpStr = "cubi";
        getSigninClass(group, "Boy", "Cub");
        getSigninClass(group, "Girl", "Cub");
        //url = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocCubiSignin";
        break;

    }


    saveSignin();

}

void MainWindow::saveSignin()
{

    QString name;
    QString grade;
    QString gender;
    QString color;
    QString signin;
    int group = 1;

    QStringList nameList;
    QStringList itemList;

    if(!m_httpStr.isEmpty())
    {

       // printf("get m_httpStr :%s \n", qPrintable(m_httpStr));
        nameList = m_httpStr.split(";");

        int len= nameList.size();
        if( len > 1){
            if( nameList[0].contains("tnt")){
                group = GROUP_TNT;
            }
            else if(nameList[0].contains("spark")){
                group = GROUP_SPARK;

            }
            else if(nameList[0].contains("cowk")){
                group = GROUP_COWK;

            }
            else if(nameList[0].contains("cubi")){

                group = GROUP_CUBI;

            }

            for(int i = 1; i< len ; i++){
                itemList = nameList[i].split(",");

                if(itemList.size() < 3 ) continue;

                name = itemList[0];
                grade = itemList[1];
                gender = itemList[2];
                color = itemList[3];
                signin = itemList[4];
                //if( group == 3) color = 'brown';

                genLocalSignin( group,  name,  grade,  gender,  color,  signin);

            }
        }


        m_httpStr="";

        m_httpCnt++;
        if( m_httpCnt >= 4){ //get all 4 group data.

            m_signNum = 0;
            leNumSignin->setText(QString("%1").arg(m_signNum));
            m_loadSignState = 2;

            QDateTime curDate = QDateTime::currentDateTime();
            QString dStr = curDate.toString("yyyy-MM-dd hh:mm:ss");

            QSqlQuery query(m_database);
            QString queryStr = QString("UPDATE SigninStatus  SET loadstatus ='%1', num2bUpd  = '%2', datetime='%3' ").arg(m_loadSignState).arg(m_signNum).arg(dStr);

            query.exec(queryStr);

            btnStartSign->setDisabled(false);
            btnSigninUpd->setDisabled(false);
            //btnLoadSignin
        }


    }
    else{
        printf(" no response \n");
    }
}

 void MainWindow::on_btnStartGame_clicked()
 {
     printf("on_btnStartGame_clicked");

         httpStartGame(GROUP_TNT);

         httpStartGame(GROUP_SPARK);

        //displayLoading(true);
 }

void MainWindow::on_btnLoadSignin_clicked()
{

    m_httpCnt= 0;

    delSignin();

    httpGetSignInMemb(GROUP_TNT);

    httpGetSignInMemb(GROUP_SPARK);

    httpGetSignInMemb(GROUP_COWK);


    httpGetSignInMemb(GROUP_CUBI);


    delBookRecord();

}


void MainWindow::addToBookDB( QString name, QString club, QString grade, QString rec)
{
    QSqlQuery query(m_database);
    QString queryStr;

   // printf("cowk: %s, %s, %s \n", qPrintable(name), qPrintable(grade), qPrintable(gender));

    queryStr = QString("INSERT INTO BookReport (name , club, grade , record ) VALUES (\'");

    queryStr += name;

    queryStr += "\', \'";
    queryStr += club;

    queryStr += "\', \'";
    queryStr += grade;

    queryStr += "\', \'";
    queryStr += rec;

    queryStr += "\')";

    if (!query.exec(queryStr))
    {
        printf("get error:%s \n", qPrintable(queryStr));
    }
}

void MainWindow::on_btnConfirmTntBook_clicked()
{
    QString rec= "";
    QString name = leNameTntBook->text();
   // QString grade = cbbGradeTntBook->currentText() ;
    QString grade = leColorTntBook->text() ;

    QSqlQuery chkQuery(m_database);
    QString sqlStr;

    sqlStr = QString("SELECT name, grade, record  FROM BookReport where club = 'tnt' and name='%1' and grade= '%2' ").arg(name).arg(grade);

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        lbTntBookStatus->setText("DB error");
        QMessageBox::information(this, tr("Error"), tr("DB Error to Add Book"));

        return;
    }

    if( chkQuery.next() )
    {
        printf("the person is already entered:%s \n", qPrintable(name));
        lbTntBookStatus->setText("the person is already entered");
        QMessageBox::information(this, tr("Error"), tr("the person is already entered"));

        return;
    }


    int len = lwTntBookRecord->count();
    if( len == 0){
        lbTntBookStatus->setText("Please enter record");
        QMessageBox::information(this, tr("Error"), tr("Please enter record"));

        return;
    }

    /*
    for( int i = 0; i< len; i++){
        QString item = lwTntBookRecord->item(i)->text();
        if( i == 0){
            rec = rec + item;
        }
        else{
            rec = rec + "," + item;
        }

    }*/
    rec = lwTntBookRecord->item(0)->text();

    addToBookDB(  name,  "tnt",  grade,  rec);
    //
    int rowCnt = tbwBookRecordTnt->rowCount();
    tbwBookRecordTnt->setRowCount(rowCnt + 1);

    tbwBookRecordTnt->setItem(rowCnt, 0, new QTableWidgetItem(name));
    tbwBookRecordTnt->setItem(rowCnt, 1, new QTableWidgetItem(grade));
    tbwBookRecordTnt->setItem(rowCnt, 2, new QTableWidgetItem(rec));

    m_curLen = 0;

    lwTntBookRecord->clear();
    //lbTntBookOK->setPixmap(QPixmap::fromImage(m_imageRed));
    lbTntBookOK->show();

    m_tntBookState  = BOOK_STATE_IDLE;
    setTntBookUI();

}

void MainWindow::on_btnClearTntBook_clicked()
{
    QString text = QString(tr("Clear all the records. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    lwTntBookRecord->clear();
    m_curLen = 0;
    m_tntBookState  = BOOK_STATE_IDLE;
    setTntBookUI();

}

void MainWindow::setTntBookUI()
{
    switch(m_tntBookState){
    case BOOK_STATE_IDLE:

        cbbNameTntBook->setDisabled(false);
        cbbGenderTntBook->setDisabled(false);
        cbbGradeTntBook->setDisabled(false);

        btnConfirmTntBook->setDisabled(false);
        btnClearTntBook->setDisabled(false);

        btnRemoveTntBook->setDisabled(false);

        cbbBookTntBook->setDisabled(false);

        cbbChapterTntBook->setDisabled(false);

        leSessionTntBook->setDisabled(false);

        break;
    case BOOK_STATE_ENTER:

        cbbNameTntBook->setDisabled(true);
        cbbGenderTntBook->setDisabled(true);
        cbbGradeTntBook->setDisabled(true);

        btnConfirmTntBook->setDisabled(true);
        btnClearTntBook->setDisabled(true);

        btnRemoveTntBook->setDisabled(true);

        cbbBookTntBook->setDisabled(true);

        cbbChapterTntBook->setDisabled(true);

        //leSessionTntBook->setDisabled(false);
        break;

    case BOOK_STATE_CHPT_DONE:
        cbbNameTntBook->setDisabled(true);
        cbbGenderTntBook->setDisabled(true);
        cbbGradeTntBook->setDisabled(true);

        btnConfirmTntBook->setDisabled(false);
        btnClearTntBook->setDisabled(false);

        btnRemoveTntBook->setDisabled(false);

        cbbBookTntBook->setDisabled(false);

        cbbChapterTntBook->setDisabled(false);

        //leSessionTntBook->setDisabled(false);
        break;
    }

}

void MainWindow::tbtBookTextChanged(const QString sess)
{
    int len = sess.length();
    if( len == 1){
        m_tntBookState = BOOK_STATE_ENTER;

    }
    else if(len == 0){
        if(0 == lwTntBookRecord->count()){
            m_tntBookState = BOOK_STATE_IDLE;

        }
        else{
            m_tntBookState = BOOK_STATE_CHPT_DONE;
        }

    }
    setTntBookUI();
}

void MainWindow::spkBookTextChanged(const QString sess)
{
    int len = sess.length();
    printf("spkBookTextChanged:%d\n", len);
    if( len == 1){
        printf("BOOK_STATE_ENTER \n");
        m_spkBookState = BOOK_STATE_ENTER;

    }
    else if(len == 0){
        if(0 == lwSpkBookRecord->count()){
            printf("BOOK_STATE_IDLE \n");
            m_spkBookState = BOOK_STATE_IDLE;

        }
        else{
            printf("BOOK_STATE_DONE:%d \n", lwSpkBook->count());
            m_spkBookState = BOOK_STATE_CHPT_DONE;
        }

    }
    setSpkBookUI();
}


void MainWindow::deleteBookrecord(QString name, QString grade, QString club)
{
    QSqlQuery query(m_database);
    QString queryStr;

    queryStr = QString("DELETE FROM BookReport where ");

    queryStr += QString("name = '%1' ").arg(name);

    queryStr += QString("and grade='%1' ").arg(grade);

    queryStr += QString("and club='%1' ").arg(club);


    if (!query.exec(queryStr))
    {
        printf("get error \n");

        qDebug() << "query failed:" << queryStr;

    }
     qDebug() << "query :" << queryStr;
    printf("exec: %s \n", qPrintable(queryStr));

}
void MainWindow::on_btnDeleteTntBook_clicked()
{

    QList<QTableWidgetItem*> items = tbwBookRecordTnt->selectedItems();
    if ( items.count() == 0 ) {

       return;
    }

    QString text = QString(tr("Delete  the record. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    int i = items[0]->row();


    QTableWidgetItem * nameItem = tbwBookRecordTnt->item ( i, 0 ) ;
    QTableWidgetItem * gradeItem = tbwBookRecordTnt->item ( i, 1 ) ;

    QString name = nameItem->text();
    QString grade = gradeItem->text();



    deleteBookrecord( name,  grade,  "tnt");
    tbwBookRecordTnt->removeRow (  i );


}

void MainWindow::httpPostBookRec( int club)
{
/*

    if(club == GROUP_TNT ){
        m_httpState = OPCODE_TNT_BOOK_REC ;

        urlStr = sslStr + "hoc5tnt-developer-edition.na14.force.com/CsvDocBookRecProc";

    }
    else if(club == GROUP_SPARK ){
        m_httpState = OPCODE_SPARK_BOOK_REC ;

        urlStr = sslStr + "hoc5sparks-developer-edition.na9.force.com/CsvDocBookRecProc";
    }
*/

}

void MainWindow::on_btnUpdDBTntBook_clicked()
{


        printf("update remote db \n");
        int cnt = tbwBookRecordTnt->rowCount();

        QTableWidgetItem * nameItem;
        QTableWidgetItem * gradeItem;
        QTableWidgetItem * recItem;

        QString name;
        QString grade;
        QString record;


        m_updateStr="tnt;";


        for( int i = 0; i< cnt ; i ++ ){
            nameItem = tbwBookRecordTnt->item ( i, 0 ) ;
            gradeItem = tbwBookRecordTnt->item ( i, 1 ) ;
            recItem = tbwBookRecordTnt->item ( i, 2 ) ;

            name = nameItem->text();
            grade = gradeItem->text();
            record = recItem->text();

            //printf("%s, %s, %s \n", qPrintable(name), qPrintable(grade), qPrintable(gender) );
            m_updateStr = m_updateStr + name + "," + grade + "," + record + ";";

        }

        m_updateStr = m_updateStr + "End_";
        printf("m_updateStr: %s \n", qPrintable(m_updateStr) );

        httpPostBookRec( GROUP_TNT);
        //displayLoading(true);


}


void MainWindow::on_cbbNameTntBook_currentIndexChanged(int i)
{
    QString name = cbbNameTntBook->currentText();
    leNameTntBook->setText(name);
    getColorTntBook(name);
    resetScoreTnt();
    leNameTntBook->setDisabled(false);
    lwTntBookRecord->clear();
    leSessionTntBook->clear();
    lbTntBookOK->hide();
    m_currBookRec="";


}

void MainWindow::on_cbbGradeTntBook_currentIndexChanged(int i)
{

    printf("on_cbbGradeTntBook_currentIndexChanged :%d \n", i);

    updateClassName( );

}

void MainWindow::on_cbbGenderTntBook_currentIndexChanged(int i)
{

    printf("on_cbbGenderTntBook_currentIndexChanged :%d \n", i);

    updateClassName( );

}

void MainWindow::updateClassName()
{
    QString grade = cbbGradeTntBook->currentText();
     QString gend = cbbGenderTntBook->currentText();

    QString name;

    QSqlQuery chkQuery(m_database);
    QString sqlStr;


    sqlStr = QString("SELECT name FROM SininMembs where grade = '%1' and gender = '%2' ").arg(grade).arg(gend);

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return;
    }

    cbbNameTntBook->clear();
    while( chkQuery.next() )
    {

        name =  chkQuery.value(0).toString();

        cbbNameTntBook->addItem(name);

    }

    name = cbbNameTntBook->currentText();
    leNameTntBook->setText(name);
    leNameTntBook->setDisabled(false);

}

void MainWindow::delBookRecord()
{
    printf("delBookRecord \n");

    QSqlQuery query(m_database);

    QString queryStr ;


    queryStr= QString("DELETE FROM BookReport");
    if (!query.exec(queryStr))
    {
        printf("delete failed:%s \n", qPrintable(queryStr));
    }



}

void  MainWindow::on_cbbBookTntBook_currentIndexChanged(int index)
{
   // QString book = cbbGenderTntBook->currentText();
    if( index == 0){
        cbbChapterTntBook->setDisabled(true);
    }
    else{
        cbbChapterTntBook->setDisabled(false);
    }

}

void MainWindow::on_btnClearLocalTntBook_clicked()
{

    QString text = QString(tr("Clear all records, including Sparks. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }

    delBookRecord();


    displayTntBookRec();

}

void MainWindow::displayTntBookRec()
{
    QString name;
    QString grade;
    QString rec;


    int cnt = tbwBookRecordTnt->rowCount();

    for( int i = 0; i< cnt ; i ++ ){
       tbwBookRecordTnt->removeRow ( cnt - i -1 );
    }

    QSqlQuery chkQuery(m_database);
    QString sqlStr;

    sqlStr = QString("SELECT name, grade, record  FROM BookReport where club = 'tnt'");

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return;
    }

    while( chkQuery.next() )
    {

        name =    chkQuery.value(0).toString();
        grade =  chkQuery.value(1).toString();
        rec =    chkQuery.value(2).toString();

        int rowCnt = tbwBookRecordTnt->rowCount();
        tbwBookRecordTnt->setRowCount(rowCnt + 1);

        tbwBookRecordTnt->setItem(rowCnt, 0, new QTableWidgetItem(name));
        tbwBookRecordTnt->setItem(rowCnt, 1, new QTableWidgetItem(grade));
        tbwBookRecordTnt->setItem(rowCnt, 2, new QTableWidgetItem(rec));

    }

}
 void MainWindow::checkNameTntBook(QString t)
 {
     QString grade = cbbGradeTntBook->currentText();
     QString gender = cbbGenderTntBook->currentText();

     QString name = t.trimmed();

    // printf("name: %s\n", qPrintable(name) );
     if( name.length()== 0) return;

     lwTntBook->clear();

     QStringList nameList;

     nameList.clear();

     QSqlQuery chkQuery(m_database);
     QString sqlStr;
     QString ggStr = QString("select name, color from SininMembs where grade='%1' and gender='%2'").arg(grade).arg(gender);

     sqlStr = QString(ggStr + " and name like '%" + name + "%'");

     if(!chkQuery.exec(sqlStr))
     {
         printf("chkQuery failed:%s \n", qPrintable(sqlStr));
         return ;
     }

     while( chkQuery.next() )
     {
         nameList << chkQuery.value(0).toString();

     }

     lwTntBook->addItems(nameList);


       if( nameList.size() == 1 && m_curLen < name.length()){
           if(m_curLen + 1 == name.length() ){
            leNameTntBook->setDisabled(true);
           }

           leNameTntBook->setText(nameList[0]);
           getColorTntBook(nameList[0]);
           resetScoreTnt();

           lwTntBookRecord->clear();
           leSessionTntBook->clear();
           lbTntBookOK->hide();
           m_currBookRec="";


           m_curLen = nameList[0].length();

       }
       else{

         m_curLen = name.length();
       }



 }

void MainWindow::tntBookNameTextChanged(const QString t)
{

    checkNameTntBook(t);

}

void MainWindow::resetScoreTnt()
{
    leSignature->setText("0");
    leExtra->setText("0");
    leNumSec->setText("0");

}

void MainWindow::getColorTntBook( QString name)
{

    //QString name = cbbNameTntBook->currentText();

    QString grade = cbbGradeTntBook->currentText();
    QString gend = cbbGenderTntBook->currentText();



    QSqlQuery chkQuery(m_database);
    QString sqlStr;


    sqlStr = QString("SELECT color FROM SininMembs where grade = '%1' and gender = '%2' and name = '%3' ").arg(grade).arg(gend).arg(name);

     if(!chkQuery.exec(sqlStr))
     {
         printf("chkQuery failed:%s \n", qPrintable(sqlStr));
         return;
     }


     if( chkQuery.next() )
     {
         QString color =  chkQuery.value(0).toString();

         leColorTntBook->setText(color);
     }

}

void MainWindow::lwTntBookItemSelected(QListWidgetItem * item)
{
    printf("lwTntBookItemSelected \n");
    QString name = item->text();

    leNameTntBook->setText(name);

    getColorTntBook(name);
    resetScoreTnt();

    lwTntBookRecord->clear();
    leSessionTntBook->clear();
    lbTntBookOK->hide();
    m_currBookRec="";

    //leNameTntBook->setDisabled(true);

    m_curLen = name.length();

}


void  MainWindow::on_btnRemoveTntBook_clicked()
{
    QString text = QString(tr("Clear all the records. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    printf("on_btnRemoveTntBook_clicked \n");
    QList<QListWidgetItem *> itemList = lwTntBookRecord->selectedItems () ;
    if( itemList.size() == 0) {
        printf("no item is selected \n");
        return;
    }
    QListWidgetItem * item = itemList.at(0);
    if( item != NULL ){
        int row = lwTntBookRecord->row(item);
        lwTntBookRecord->takeItem ( row );
    }
    else
        printf(" item != NULL \n");

}



void MainWindow::setSpkBookUI()
{
    switch(m_spkBookState){
    case BOOK_STATE_IDLE:

        cbbNameSpkBook->setDisabled(false);
        cbbGenderSpkBook->setDisabled(false);
        cbbGradeSpkBook->setDisabled(false);

        btnConfirmSpkBook->setDisabled(false);
        btnClearSpkBook->setDisabled(false);

        btnRemoveSpkBook->setDisabled(false);

        cbbBookSpkBook->setDisabled(false);

        cbbChapterSpkBook->setDisabled(false);

        leSessionSpkBook->setDisabled(false);

        break;
    case BOOK_STATE_ENTER:

        cbbNameSpkBook->setDisabled(true);
        cbbGenderSpkBook->setDisabled(true);
        cbbGradeSpkBook->setDisabled(true);

        btnConfirmSpkBook->setDisabled(true);
        btnClearSpkBook->setDisabled(true);

        btnRemoveSpkBook->setDisabled(true);

        cbbBookSpkBook->setDisabled(true);

        cbbChapterSpkBook->setDisabled(true);

        //leSessionSpkBook->setDisabled(false);
        break;

    case BOOK_STATE_CHPT_DONE:
        cbbNameSpkBook->setDisabled(true);
        cbbGenderSpkBook->setDisabled(true);
        cbbGradeSpkBook->setDisabled(true);

        btnConfirmSpkBook->setDisabled(false);
        btnClearSpkBook->setDisabled(false);

        btnRemoveSpkBook->setDisabled(false);

        cbbBookSpkBook->setDisabled(false);

        cbbChapterSpkBook->setDisabled(false);

        //leSessionSpkBook->setDisabled(false);
        break;
    }

}




void MainWindow::on_btnConfirmSpkBook_clicked()
{
    QString rec= "";
    QString name = leNameSpkBook->text();
    QString grade = leColorSpkBook->text(); //cbbGradeSpkBook->currentText() ;

    QSqlQuery chkQuery(m_database);
    QString sqlStr;

    sqlStr = QString("SELECT name, grade, record  FROM BookReport where club = 'spk' and name='%1' and grade= '%2' ").arg(name).arg(grade);

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        QMessageBox::information(this, tr("Error"), tr("DB Error to Add Book"));

        return;
    }

    if( chkQuery.next() )
    {
        printf("the person is already entered:%s \n", qPrintable(name));
        QMessageBox::information(this, tr("Error"), tr("the person is already entered"));

        return;
    }


    int len = lwSpkBookRecord->count();

    if( len ==0 ) {
        QMessageBox::information(this, tr("Error"), tr("Please Enter a Book"));

        return;
    }
/*
    for( int i = 0; i< len; i++){
        QString item = lwSpkBookRecord->item(i)->text();
        if( i == 0){
            rec = rec + item;
        }
        else{
            rec = rec + "," + item;
        }

    }
*/
    rec = lwSpkBookRecord->item(0)->text();

    addToBookDB(  name,  "spk",  grade,  rec);
    //
    int rowCnt = tbwBookRecordSpk->rowCount();
    tbwBookRecordSpk->setRowCount(rowCnt + 1);

    tbwBookRecordSpk->setItem(rowCnt, 0, new QTableWidgetItem(name));
    tbwBookRecordSpk->setItem(rowCnt, 1, new QTableWidgetItem(grade));
    tbwBookRecordSpk->setItem(rowCnt, 2, new QTableWidgetItem(rec));

    m_curLen = 0;

    lwSpkBookRecord->clear();
    lbSpkBookOK->show();

    printf("BOOK_STATE_IDLE \n");
    m_spkBookState  = BOOK_STATE_IDLE;
    setSpkBookUI();
}

void MainWindow::on_btnClearSpkBook_clicked()
{
    QString text = QString(tr("Clear all the records. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    lwSpkBookRecord->clear();
     m_curLen = 0;

     printf("BOOK_STATE_IDLE \n");
     m_spkBookState  = BOOK_STATE_IDLE;
     setSpkBookUI();
}

void MainWindow::on_btnDeleteSpkBook_clicked()
{
    QList<QTableWidgetItem*> items = tbwBookRecordSpk->selectedItems();
    if ( items.count() == 0 ) {

           return;
    }

    QString text = QString(tr("Delete  the record. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    int i = items[0]->row();


    QTableWidgetItem * nameItem = tbwBookRecordSpk->item ( i, 0 ) ;
    QTableWidgetItem * gradeItem = tbwBookRecordSpk->item ( i, 1 ) ;

    QString name = nameItem->text();
    QString grade = gradeItem->text();

    deleteBookrecord( name,  grade,  "spk");

    tbwBookRecordSpk->removeRow (  i );

}


void MainWindow::on_btnUpdDBSpkBook_clicked()
{


        printf("update remote db \n");
        int cnt = tbwBookRecordSpk->rowCount();

        QTableWidgetItem * nameItem;
        QTableWidgetItem * gradeItem;
        QTableWidgetItem * recItem;

        QString name;
        QString grade;
        QString record;


        m_updateStr="spark;";


        for( int i = 0; i< cnt ; i ++ ){
            nameItem = tbwBookRecordSpk->item ( i, 0 ) ;
            gradeItem = tbwBookRecordSpk->item ( i, 1 ) ;
            recItem = tbwBookRecordSpk->item ( i, 2 ) ;

            name = nameItem->text();
            grade = gradeItem->text();
            record = recItem->text();

            //printf("%s, %s, %s \n", qPrintable(name), qPrintable(grade), qPrintable(gender) );
            m_updateStr = m_updateStr + name + "," + grade + "," + record + ";";

        }

        m_updateStr = m_updateStr + "End_";
        printf("m_updateStr: %s \n", qPrintable(m_updateStr) );

        httpPostBookRec( GROUP_SPARK);
        //displayLoading(true);


}


void MainWindow::on_cbbNameSpkBook_currentIndexChanged(int i)
{
    QString name = cbbNameSpkBook->currentText();

    leNameSpkBook->setText(name);
    getColorSpkBook(name);
    resetScoreSpk();
    lwSpkBookRecord->clear();
    leSessionSpkBook->clear();
    lbSpkBookOK->hide();
    m_currBookRec="";


}

void MainWindow::on_cbbGradeSpkBook_currentIndexChanged(int i)
{

    printf("on_cbbGradeSpkBook_currentIndexChanged :%d \n", i);

    updateSpkClassName( );

}

void MainWindow::on_cbbGenderSpkBook_currentIndexChanged(int i)
{

    printf("on_cbbGenderTntBook_currentIndexChanged :%d \n", i);

    updateSpkClassName( );

}

void MainWindow::updateSpkClassName()
{
    QString grade = cbbGradeSpkBook->currentText();
     QString gend = cbbGenderSpkBook->currentText();

    QString name;

    QSqlQuery chkQuery(m_database);
    QString sqlStr;


    sqlStr = QString("SELECT name FROM SparkSigninMembs where grade = '%1' and gender = '%2' ").arg(grade).arg(gend);

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return;
    }

    cbbNameSpkBook->clear();
    while( chkQuery.next() )
    {

        name =  chkQuery.value(0).toString();

        cbbNameSpkBook->addItem(name);

    }

    name = cbbNameSpkBook->currentText();
    leNameSpkBook->setText(name);
    leNameSpkBook->setDisabled(false);

}

void MainWindow::resetScoreSpk()
{
    leSignatureSpk->setText("0");
    leExtraSpk->setText("0");
    leNumSecSpk->setText("0");

}

void MainWindow::getColorSpkBook( QString name)
{

    //QString name = cbbNameTntBook->currentText();

    QString grade = cbbGradeSpkBook->currentText();
    QString gend = cbbGenderSpkBook->currentText();


    QSqlQuery chkQuery(m_database);
    QString sqlStr;


    sqlStr = QString("SELECT color FROM SparkSigninMembs where grade = '%1' and gender = '%2' and name = '%3' ").arg(grade).arg(gend).arg(name);

     if(!chkQuery.exec(sqlStr))
     {
         printf("chkQuery failed:%s \n", qPrintable(sqlStr));
         return;
     }


     if( chkQuery.next() )
     {
         QString color =  chkQuery.value(0).toString();

         leColorSpkBook->setText(color);
     }


}

void MainWindow::on_cbbBookSpkBook_currentIndexChanged(int index)
{
    switch(index){
    case 0:
    case 2:
    case 4:
    case 6:
        cbbChapterSpkBook->setDisabled(true);

        break;

    default:
        cbbChapterSpkBook->setDisabled(false);
        break;
    }

}

void MainWindow::on_btnClearLocalSpkBook_clicked()
{

    QString text = QString(tr("Clear all the records, including TNT's. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }

    printf("on_btnClearLocalSpkBook_clicked\n");

    delBookRecord();


    displaySpkBookRec();

}

void MainWindow::displaySpkBookRec()
{
    QString name;
    QString grade;
    QString rec;


    int cnt = tbwBookRecordSpk->rowCount();

    for( int i = 0; i< cnt ; i ++ ){
       tbwBookRecordSpk->removeRow ( cnt - i -1 );
    }

    QSqlQuery chkQuery(m_database);
    QString sqlStr;

    sqlStr = QString("SELECT name, grade, record  FROM BookReport where club = 'spk'");

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        return;
    }

    while( chkQuery.next() )
    {

        name =    chkQuery.value(0).toString();
        grade =  chkQuery.value(1).toString();
        rec =    chkQuery.value(2).toString();

        int rowCnt = tbwBookRecordSpk->rowCount();
        tbwBookRecordSpk->setRowCount(rowCnt + 1);

        tbwBookRecordSpk->setItem(rowCnt, 0, new QTableWidgetItem(name));
        tbwBookRecordSpk->setItem(rowCnt, 1, new QTableWidgetItem(grade));
        tbwBookRecordSpk->setItem(rowCnt, 2, new QTableWidgetItem(rec));

    }

}
 void MainWindow::checkNameSpkBook(QString t)
 {
     QString grade = cbbGradeSpkBook->currentText();
     QString gender = cbbGenderSpkBook->currentText();

     QString name = t.trimmed();

    // printf("name: %s\n", qPrintable(name) );
     if( name.length()== 0) return;

     lwSpkBook->clear();

     QStringList nameList;

     nameList.clear();

     QSqlQuery chkQuery(m_database);
     QString sqlStr;
     QString ggStr = QString("select name from SparkSigninMembs where grade='%1' and gender='%2'").arg(grade).arg(gender);

     sqlStr = QString(ggStr + " and name like '%" + name + "%'");

     if(!chkQuery.exec(sqlStr))
     {
         printf("chkQuery failed:%s \n", qPrintable(sqlStr));
         return ;
     }

     while( chkQuery.next() )
     {
         nameList << chkQuery.value(0).toString();

     }

     lwSpkBook->addItems(nameList);


       if( nameList.size() == 1 && m_curLen < name.length()){
           if(m_curLen + 1 == name.length()){

               leNameSpkBook->setDisabled(true);
           }
           leNameSpkBook->setText(nameList[0]);
           getColorSpkBook(nameList[0]);
           resetScoreSpk();

           lwSpkBookRecord->clear();
           leSessionSpkBook->clear();
           lbSpkBookOK->hide();
           m_currBookRec="";



           m_curLen = nameList[0].length();

       }
       else{

         m_curLen = name.length();
       }



 }

void MainWindow::spkBookNameTextChanged(const QString t)
{

    checkNameSpkBook(t);

}
void MainWindow::lwSpkBookItemSelected(QListWidgetItem * item)
{
    printf("lwTntBookItemSelected \n");
    QString name = item->text();

    leNameSpkBook->setText(name);
    getColorSpkBook(name);
    resetScoreSpk();

    lwSpkBookRecord->clear();
    leSessionSpkBook->clear();
    lbSpkBookOK->hide();
    m_currBookRec="";

    leNameSpkBook->setDisabled(true);

    m_curLen = name.length();

}


void  MainWindow::on_btnRemoveSpkBook_clicked()
{
    QString text = QString(tr("Remove all the records. Continue?"));
    int rtn = QMessageBox::question(this, tr("Confirmation"), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if ( rtn == QMessageBox::No )
    {
        return;
    }
    printf("on_btnRemoveTntBook_clicked \n");
    QList<QListWidgetItem *> itemList = lwSpkBookRecord->selectedItems () ;
    if( itemList.size() == 0) {
        printf("no item is selected \n");
        return;
    }
    QListWidgetItem * item = itemList.at(0);
    if( item != NULL ){
        int row = lwSpkBookRecord->row(item);
        lwSpkBookRecord->takeItem ( row );
    }
    else
        printf(" item != NULL \n");

}


void MainWindow::on_btnScoreSpk_clicked()
{
    bool ok=false;
    int total[4];
    int score[4];
    total[0] = leRedScoreSpk->text().toInt(&ok);
    if( ok == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Score for Red "));

    }

    total[1] = leBlueScoreSpk->text().toInt(&ok);
    if( ok == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Score for Blue "));

    }

    total[2] =  leGreenScoreSpk->text().toInt(&ok);
    if( ok == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Score for Green "));

    }

    total[3] = leYellowScoreSpk->text().toInt(&ok);
    if( ok == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Score for Yellow "));

    }

    total[0] = total[0] + getBookScore("spk", "Red");
    total[1] = total[1] + getBookScore("spk", "Blue");
    total[2] = total[2] + getBookScore("spk", "Green");
    total[3] = total[3] + getBookScore("spk", "Yellow");

    leRedScoreSpk->setText(QString("%1").arg(total[0]));
    leGreenScoreSpk->setText(QString("%1").arg(total[2]));
    leBlueScoreSpk->setText(QString("%1").arg(total[1]));
    leYellowScoreSpk->setText(QString("%1").arg(total[3]));

    for(int i = 0; i< 4; i ++){
        score[i] = 4;
    }
    int max = total[0];
    int maxIndex = 0;
    for(int i = 1; i< 4; i ++){
        if( total[i] > max){
            max = total[i];
            maxIndex = i;
        }
    }
    score[maxIndex]= 1;
    total[maxIndex] = 0;

    max = total[0];
    maxIndex = 0;
    for(int i = 1; i< 4; i ++){
        if( total[i] > max){
            max = total[i];
            maxIndex = i;
        }
    }
    score[maxIndex]= 2;
    total[maxIndex] = 0;

    max = total[0];
    maxIndex = 0;
    for(int i = 1; i< 4; i ++){
        if( total[i] > max){
            max = total[i];
            maxIndex = i;
        }
    }
    score[maxIndex]= 3;
    total[maxIndex] = 0;

    lbRedFinalSpk->setText(QString("%1").arg(score[0]));
    lbGreenFinalSpk->setText(QString("%1").arg(score[2]));
    lbBlueFinalSpk->setText(QString("%1").arg(score[1]));
    lbYellowFinalSpk->setText(QString("%1").arg(score[3]));
}

int MainWindow::genScoreSpk()
{
    //QString ret="";
    bool isOK;
    int signature = leSignatureSpk->text().toInt(&isOK);
    if( isOK == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Signature credit "));
        return -1;

    }
    int extra = leExtraSpk->text().toInt(&isOK);
    if( isOK == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Extra credit "));
        return -1;

    }
    int numSec = leNumSecSpk->text().toInt(&isOK);
    if( isOK == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong data for number of sessions "));
        return -1;

    }
    if( numSec > 10){
        QMessageBox::information(this, tr("Error"), tr("??? data for number of sessions "));

    }

     int total = signature + extra;
     switch(numSec){
     case 0:
         return total;
     case 1:
         return total + 500;
     //case 2:
        // return total + 500;
     default:

         return total + 1000 * (numSec);
     }
}
QString MainWindow::genSpkBookRec()
{
    QString ret="";
    QString session = leSessionSpkBook->text();

    //QString grade = cbbGradeSpkBook->currentIndex(); //currentText();
    int bk = cbbBookSpkBook->currentIndex();
    int ch = cbbChapterSpkBook->currentIndex();

    if(bk == 0) ch = 0;

    QString book="";
    QString chap="";

    book = QString("%1").arg(bk);
    chap = QString("%1").arg(ch);


  /*
    if(bk.contains("Flight")){
        book = "0";
        chap= "0";
    }
    else if( bk.contains("Glider F")){
         book = "2";
    }
    else if( bk.contains("Runner F")){
         book = "4";
    }
    else if( bk.contains("Stormer F")){
         book = "6";
    }
    else if( bk.contains("Glider")){
         book = "1";
    }
    else if( bk.contains("Runner")){
         book = "3";
    }
    else if( bk.contains("Stormer")){
         book = "5";
    }
*/


   // for (int i = 0; i< len; i++)
    {
        if(bk== 0){
            int len = session.length();
            for (int i = 0; i< len; i++) {
                QChar s = session.at(i);
                int d =  s.digitValue();
                if( d < 1 || d > 6 ) return "";
                if(i == 0){
                     ret= ret + book + "_" + chap + "_" + s;
                }
                else{
                    ret= ret + "," + book + "_" + chap + "_" + s;
                }
            }

        }
        else if( bk == 1 || bk == 3 || bk == 5){
            if( ch == 0 || ch == 5 ){
                int len = session.length();
                for (int i = 0; i< len; i++) {
                    QChar s = session.at(i);
                    int d =  s.digitValue();
                    if( d < 1 || d > 8 ) return "";

                    if(i == 0){
                         ret= ret + book + "_" + chap + "_" + s;
                    }
                    else{
                        ret= ret + "," + book + "_" + chap + "_" + s;
                    }
                }


            }
            else{
                QStringList list = session.split(",");
                int len = list.size();
                for (int i = 0; i < len; i ++){
                    QString s="";

                    if( list.at(i).contains("r1")){
                        s="1";
                    }
                    else if( list.at(i).contains("r2")){
                        s="2";
                    }
                    else if( list.at(i).contains("r3")){
                        s="3";
                    }
                    else if( list.at(i).contains("r4")){
                        s="4";
                    }
                    else if( list.at(i).contains("g1")){
                        s="5";
                    }
                    else if( list.at(i).contains("g2")){
                        s="6";
                    }
                    else if( list.at(i).contains("g3")){
                        s="7";
                    }
                    else if( list.at(i).contains("g4")){
                        s="8";
                    }
                    else{
                        return "";
                    }

                    if(i == 0){
                         ret= ret + book + "_" + chap + "_" + s;
                    }
                    else{
                        ret= ret + "," + book + "_" + chap + "_" + s;
                    }
                }

            }
        }
        else{
            QStringList list = session.split(",");
            int len = list.size();
            for (int i = 0; i < len; i ++){

                QString item = list.at(i);
                bool ok;
                int sess = item.toInt(&ok, 10);
                if( ok == false) return "";
                if( sess > 80) return "";

                int c = sess/8;
                if( c > 9) return "";
                int ss = sess % 8;
                if( ss == 0) ss = 8;

                if(i == 0){
                     ret= ret + book + "_" + QString("%1_%2").arg(c).arg(ss);
                }
                else{
                    ret= ret + "," + book + "_" + QString("%1_%2").arg(c).arg(ss);
                }

            }
        }

    }

    return ret;

}

void MainWindow::on_btnScoreTnt_clicked()
{
    bool ok=false;
    int total[4];
    int score[4];
    total[0] = leRedScore->text().toInt(&ok);
    if( ok == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Score for Red "));

    }

    total[2] =  leGreenScore->text().toInt(&ok);
    if( ok == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Score for Green "));

    }

    total[1] = leBlueScore->text().toInt(&ok);
    if( ok == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Score for Blue "));

    }

    total[3] = leYellowScore->text().toInt(&ok);
    if( ok == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Score for Yellow "));

    }

    total[0] = total[0] + getBookScore("tnt", "Red");
    total[1] = total[1] + getBookScore("tnt", "Blue");
    total[2] = total[2] + getBookScore("tnt", "Green");
    total[3] = total[3] + getBookScore("tnt", "Yellow");

    leRedScore->setText(QString("%1").arg(total[0]));
    leGreenScore->setText(QString("%1").arg(total[2]));
    leBlueScore->setText(QString("%1").arg(total[1]));
    leYellowScore->setText(QString("%1").arg(total[3]));

    for(int i = 0; i< 4; i ++){
        score[i] = 4;
    }

    int max = total[0];
    int maxIndex = 0;
    for(int i = 1; i< 4; i ++){
        if( total[i] > max){
            max = total[i];
            maxIndex = i;
        }
    }
    score[maxIndex]= 1;
    total[maxIndex] = 0;

    max = total[0];
    maxIndex = 0;
    for(int i = 1; i< 4; i ++){
        if( total[i] > max){
            max = total[i];
            maxIndex = i;
        }
    }
    score[maxIndex]= 2;
    total[maxIndex] = 0;

    max = total[0];
    maxIndex = 0;
    for(int i = 1; i< 4; i ++){
        if( total[i] > max){
            max = total[i];
            maxIndex = i;
        }
    }
    score[maxIndex]= 3;
    total[maxIndex] = 0;

    lbRedFinal->setText(QString("%1").arg(score[0]));
    lbGreenFinal->setText(QString("%1").arg(score[2]));
    lbBlueFinal->setText(QString("%1").arg(score[1]));
    lbYellowFinal->setText(QString("%1").arg(score[3]));
}

int MainWindow::getBookScore(QString club, QString color)
{
    //'tnt'
    QSqlQuery chkQuery(m_database);
    QString sqlStr;

    sqlStr = QString("SELECT record  FROM BookReport where club = '%1' and grade= '%2' ").arg(club).arg(color);

    if(!chkQuery.exec(sqlStr))
    {
        printf("chkQuery failed:%s \n", qPrintable(sqlStr));
        lbTntBookStatus->setText("DB error");
        QMessageBox::information(this, tr("Error"), tr("DB Error to Add Book"));

        return 0;
    }

    int score = 0;
    while( chkQuery.next() )
    {
        score = score +  chkQuery.value(0).toString().toInt();


    }

    return score;

}

int MainWindow::genScore()
{
    //QString ret="";
    bool isOK;
    int signature = leSignature->text().toInt(&isOK);
    if( isOK == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong signature credit "));
        return -1;

    }
    int extra = leExtra->text().toInt(&isOK);
    if( isOK == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong Extra credit "));
        return -1;

    }
    int numSec = leNumSec->text().toInt(&isOK);
    if( isOK == false) {
        QMessageBox::information(this, tr("Error"), tr("Wrong data for number of sessions "));
        return -1;

    }
    if( numSec > 10){
        QMessageBox::information(this, tr("Error"), tr("??? data for number of sessions "));

    }

     int total = signature + extra;
     switch(numSec){
     case 0:
         return total;
     case 1:
         return total + 500;
     //case 2:
       //  return total + 500;
     default:

         return total + 1000 * (numSec );
     }

}


QString MainWindow::genBookRec()  //not used
{
    QString ret="";
    QString session = leSessionTntBook->text();

    QString grade = cbbGradeTntBook->currentText();
    QString bk = cbbBookTntBook->currentText();
    QString ch = cbbChapterTntBook->currentText();
    /*
    if( ch.contains("S")){
        ch = "8";
    }
    else if( ch.contains("G")){
        ch= "9";
    }*/

    QString book="";

    if( grade.contains("3rd") || grade.contains("4th")){
        if(bk.contains("StartZone")){
            book = "1";
            ch= "1";
        }
        else if( bk.contains("Book 1")){
             book = "2";
        }
        else if( bk.contains("Book 2")){
             book = "3";
        }
    }

    else if( grade.contains("5th") || grade.contains("6th")){
        if(bk.contains("StartZone")){
            book = "4";
             ch= "1";
        }
        else if( bk.contains("Book 1")){
             book = "5";
        }
        else if( bk.contains("Book 2")){
             book = "6";
        }
    }
    int len = session.length();

    if( len == 0){
        lbTntBookStatus->setText("No data");
    }
    for (int i = 0; i< len; i++){
        QChar s = session.at(i);
        int d=  s.digitValue();
        if( d < 1 ) {
            lbTntBookStatus->setText("<1 ");
            return "";
        }

        if(i == 0){
             ret= ret + book + "_" + ch + "_" + s;
        }
        else{
            ret= ret + "," + book + "_" + ch + "_" + s;
        }

    }

    return ret;

}



void MainWindow::on_btnImport_clicked()
{

    QString folder = QDir::rootPath();

    QString srcFile = QFileDialog::getOpenFileName(this, tr("Get Source DB File"), folder, tr("DB File (*.db)"));

    if ( srcFile.isEmpty() )
    {

        return;

    }

    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);


    QString fileSelected = dbPath + "/awana.db";


    m_database.close();
    if( false == QFile::copy (  srcFile, fileSelected)){
        printf("QFile::copy error \n");
    }

    m_database = QSqlDatabase::database();

}


void MainWindow::on_btnExport_clicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Select Destination folder"), QDir::rootPath());

    if (folder.isEmpty()) {

        return;
    }

    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);


    QString fileSelected = dbPath + "/awana.db";

     QString newFile = folder +  "/awana.db"  ;

    m_database.close();
    if( false == QFile::copy ( fileSelected, newFile)){
        printf("QFile::copy error \n");
    }

    m_database = QSqlDatabase::database();

}


void MainWindow::on_tabCurrentChanged(int i)
{
    printf("on_tabCurrentChanged:%d,%s \n", i, qPrintable(tabWidget->tabText(i)));

    m_curLen = 0;
    if ( tabWidget->currentWidget() == tabTntBook)
    {
          printf("on_tabCurrentChanged: for tnt book \n");
         // on_btnLoadSignin_clicked();


          updateClassName( );

          int rowCnt = tbwBookRecordTnt->rowCount();
          if(rowCnt == 0){

              tbwBookRecordTnt->setColumnWidth(0,  100);
              tbwBookRecordTnt->setColumnWidth(1,  60);
              tbwBookRecordTnt->setColumnWidth(2,  340);

              displayTntBookRec();
                m_curLen = 0;

              lwTntBookRecord->clear();
          }
   }
    else if(tabWidget->currentWidget() == tabSpkBook)
    {


        printf("on_tabCurrentChanged: for tnt book \n");
       // on_btnLoadSignin_clicked();


        updateSpkClassName( );

        int rowCnt = tbwBookRecordSpk->rowCount();
        if(rowCnt == 0){

            tbwBookRecordSpk->setColumnWidth(0,  100);
            tbwBookRecordSpk->setColumnWidth(1,  60);
            tbwBookRecordSpk->setColumnWidth(2,  340);

            displaySpkBookRec();
              m_curLen = 0;

            lwSpkBookRecord->clear();
        }
    }
    else if(tabWidget->currentWidget() == tabOverview ){
        showMemberOverview();
    }

}



void MainWindow::keyPressEvent(QKeyEvent* event)
{

    QString newChar = event->text();
   // printf("main win key press:%s \n",qPrintable(newChar));

    if( newChar == "\n" || newChar == "\r"){

        if ( tabWidget->currentWidget() == tabTntBook)
        {
            //printf("got a record \n");
            int score = genScore();
            if( score <= 0 ) return;

            QString session = QString("%1").arg(score) ; //genBookRec();


            if(!session.isEmpty()){
                lwTntBookRecord->addItem(session);

                /*
                int len = lwTntBookRecord->count();
                QString bc = session.left(3);
                for(int i = 0; i < len; i++){
                    QString item = lwTntBookRecord->item(i)->text();
                    if( item.left(3).contains(bc)){
                        lbTntBookStatus->setText(QString("duplicate book and chapter: %1").arg(bc));
                        printf("duplicate book and chapter \n");
                        QMessageBox::information(this, tr("Error"), tr(" duplicate book and chapter "));

                        if( m_tntBookState == BOOK_STATE_ENTER){
                            m_tntBookState =  BOOK_STATE_CHPT_DONE;
                            setTntBookUI();
                        }
                        return;
                    }

                }

                lwTntBookRecord->addItem(session);

                if( m_tntBookState == BOOK_STATE_ENTER){
                    m_tntBookState =  BOOK_STATE_CHPT_DONE;
                    setTntBookUI();
                }*/

                //leSessionTntBook->setText("");
            }
            else{
                QMessageBox::information(this, tr("Error"), tr("Wrong Book Record "));

               // lbTntBookStatus->setText("no entry");
            }
        }
        else if(tabWidget->currentWidget() == tabSpkBook)
        {
            //QString session = genSpkBookRec();
            int score = genScoreSpk();
            if( score <= 0 ) return;
            QString session = QString("%1").arg(score) ; //genBookRec();


            if(!session.isEmpty()){
                lwSpkBookRecord->addItem(session);
                /*
                int len = lwSpkBookRecord->count();
                 QString bc = session.left(3);

                for(int i = 0; i < len; i++){
                    QString item = lwSpkBookRecord->item(i)->text();
                    if( item.left(3).contains(bc)){
                        lbTntBookStatus->setText("duplicate book and chapter");
                        QMessageBox::information(this, tr("Error"), tr(" duplicate book and chapter "));


                        if( m_spkBookState == BOOK_STATE_ENTER){
                             printf("BOOK_STATE_CHPT_DONE \n");
                            m_spkBookState =  BOOK_STATE_CHPT_DONE;
                            setSpkBookUI();
                        }
                        printf("duplicate book and chapter \n");
                        return;
                    }

                }

                lwSpkBookRecord->addItem(session);

                if( m_spkBookState == BOOK_STATE_ENTER){
                     printf("BOOK_STATE_CHPT_DONE \n");
                    m_spkBookState =  BOOK_STATE_CHPT_DONE;
                    setSpkBookUI();
                }*/
                //leSessionSpkBook->setText("");
            }
            else{
                QMessageBox::information(this, tr("Error"), tr("Wrong Book Record "));

                //lbTntBookStatus->setText("no entry");
            }
        }


    }

}


#if 0

int index = tabWidget->indexOf(tabArchivedLogs);
  tabWidget->setCurrentIndex(5);
 tabWidget->setTabEnabled(twLogs->indexOf(tabArchivedLogs), false);



#endif


