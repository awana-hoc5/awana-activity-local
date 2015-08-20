#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "ui_mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDateTime>
//#include <QPixmap>

//#include "groupwindow.h"

/*namespace Ui {
    class MainWindow;
}*/

//#include "serialinterface.h"

enum{
    BOOK_STATE_IDLE,
    BOOK_STATE_ENTER,
    BOOK_STATE_CHPT_DONE,
    BOOK_STATE_MAX

};


enum{
    MEMBER_NEW = 0,
    MEMBER_OLD,

    MEMBER_MAX

};

enum{
    GROUP_TNT = 1,
    GROUP_SPARK,
    GROUP_COWK,
    GROUP_CUBI,
    GROUP_MAX

};

enum{
    OPCODE_GET_EXIST_MEMBER = 1,
    OPCODE_ADD_NEW = 2,
    OPCODE_INACT_MEMBER=3,
    OPCODE_TNT_NEW_SIGNIN=4,
    OPCODE_SPARK_NEW_SIGNIN=5,
    OPCODE_UPDATE_COWORK= 6,
    OPCODE_POST_SIGNIN= 7,
    OPCODE_POST_SiGNOUT = 8,
    OPCODE_GET_SIGNIN_MEMBER = 10,
    OPCODE_START_GAME = 11,
    OPCODE_TNT_BOOK_REC = 12,
    OPCODE_SPARK_BOOK_REC = 13,

    OPCODE_MAX

};

class MainWindow : public QMainWindow,private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:

    void keyPressEvent(QKeyEvent* event);

private:
    int initDB();
    void checkSettings();
    void setSetting();
    void getSetting();
    void showMember(int old);
    void checkDup(int old);


    void httpGetSignInMemb(int group);
    void httpStartGame(int group);

    void httpPostSignInMemb(int group , QString name , QString  sunday , QString late , QString uniform , QString badge);
    void getSigninClass(int group, QString gender, QString grade );
    void httpPostBookRec( int club);
    void deleteTable(int old);
    void addNewMember(QString gender,QString grade,QString name,QString active);
    void saveSignin();
    void foundRemoveMemb();
    void foundNewMemb();
    void mySleep (int milliseconds);
    void displayLoading(bool yes);
    //void addNewMember();

    void deactMember(QString name, QString grade, QString gender);
    void showMemberOverview();

    QString getGender();
    QString getGrade();
    QString getBook(QString grade);
    QString getColor();
    QString getSunday();
    void resetNewSign();
    void cleanSigninTable();
    void retrieveSignin(bool isAll);
    void addSigninTable(QString name,QString grade,QString gender,QString color,QString sunday,QString book,QString update);
    void delSigninDB(QString name, QString grade, QString gender);
    void updateNewSignin(QString name, QString grade, QString gender);
    void doRbChange();

    void updateLocalSignin(int group, QString name, QString sunday, QString uniform, QString late, QString badge);
    void genLocalSignin(int group, QString name, QString grade, QString gender, QString color, QString signin);
    void delSignin();

    void upgradeOneGrade(QString grade);
    void deleteOneGrade(int group);

    QString parseFormatStr(const QString& dt);
    void addOnSiteMember(int group, QString gender,QString grade,QString name,QString active, QString color, QString sunday);


    void updateClassName();
    void checkNameTntBook(QString t);
    QString genBookRec();

    void addToBookDB( QString name, QString club, QString grade, QString rec);
    void LoadInit();
    void SaveInit();
    void displayTntBookRec();

    void updateSpkClassName();
    void checkNameSpkBook(QString t);
    QString genSpkBookRec();

    void displaySpkBookRec();
    void deleteBookrecord(QString name, QString grade, QString club);

    void delBookRecord();
    void setTntBookUI();
    void setSpkBookUI();

    int genScore();
    void getColorTntBook(QString name);

    int genScoreSpk();
    void getColorSpkBook(QString name);
    int getBookScore(QString club, QString color);
    void resetScoreSpk();
    void resetScoreTnt();

    void colorMembClass(int group, QString gender, QString grade);
    int getColorNumber(int c);

    //GroupWindow * m_groupWindow;

   // Ui::MainWindow *ui;
    QSqlDatabase m_database;

    QSqlQueryModel m_tableModel;
    QSortFilterProxyModel m_sortedModel;

    QSqlQueryModel m_inactTableModel;
    QSortFilterProxyModel m_inactSortedModel;

    QSqlQueryModel m_newTableModel;
    QSortFilterProxyModel m_newSortedModel;


    QString m_httpStr;

    int m_numOldMemb;
    int m_numNewMemb;
    int m_timerCnt;

    QTimer * m_timer;
    QString m_updateStr;

    int m_httpState;
    int m_httpCnt;

    QDateTime m_loadSignDate;
    int m_loadSignState;
    int m_signNum;

    int m_initDB;
    int m_scanOther;
    int m_scanSignin;
    int m_scanSignout;

    int m_numSignout;
      int m_curLen;
      QString m_currBookRec;

      int m_tntBookState;
      int m_spkBookState;



    //SerialInterface* m_deviceThread;

signals:

    void openSerial();
    void closeSerial();

private slots:

    void onTimeout();

    //void onReceiveBarCode(QString code);


    void tbtBookTextChanged(const QString);
    void spkBookTextChanged(const QString);

    void on_rbTntChange(bool s);
    void on_rbSparkChange(bool s);
    void on_rbCowkChange(bool s);
    void on_rbCubiChange(bool s);

    void on_rbTntOverviewChange(bool s);
    void on_rbSparkOverviewChange(bool s);
    void on_rbCowkOverviewChange(bool s);
    void on_rbCubiOverviewChange(bool s);


    void slotGotSignin(QString name, QString color, int group, bool isSunday, bool isUniform, bool isLate, bool isBadge);

    void on_btnLocal_clicked();
    void on_btnRemote_clicked();
    void on_btnUpdateNew_clicked();
    void on_btnUpdateOld_clicked();
    void on_btnDiff_clicked();

    void on_btnUpdateCowk_clicked();
    void on_btnNewGrade_clicked();



    void on_btnAddSignin_clicked();
    void on_btnDelSignin_clicked();
    void on_btnDelSigninSpark_clicked();
    void on_btnUpdateNewSignin_clicked();
    void on_btnAllSignin_clicked();
    void on_btnHideUpDB_clicked();

    void on_btnStartGame_clicked();
    void on_btnLoadSignin_clicked();
    void on_btnStartSign_clicked();
    void on_btnSigninUpd_clicked();

    void on_btnColorTeams_clicked();


    //void on_btnStartSignout_clicked();
    //void on_btnUpdSignout_clicked();


    void on_cbbNameTntBook_currentIndexChanged(int i);
    void on_cbbGradeTntBook_currentIndexChanged(int index);
    void on_cbbGenderTntBook_currentIndexChanged(int index);
    void on_cbbBookTntBook_currentIndexChanged(int index);

    void tntBookNameTextChanged(QString tc);
    //void tntBookSessionTextChanged(QString tc);
    void lwTntBookItemSelected(QListWidgetItem * item);

    void on_btnConfirmTntBook_clicked();
    void on_btnRemoveTntBook_clicked();
     void on_btnClearTntBook_clicked();

    void on_btnDeleteTntBook_clicked();
    void on_btnUpdDBTntBook_clicked();

    void on_btnClearLocalTntBook_clicked();
    void on_btnImport_clicked();
    void on_btnExport_clicked();

    //void on_tabWidget_currentChanged(QWidget* newTab);
    void on_tabCurrentChanged(int i);


    void on_btnConfirmSpkBook_clicked();
    void on_btnRemoveSpkBook_clicked();
    void on_btnClearSpkBook_clicked();

    void on_btnDeleteSpkBook_clicked();
    void on_btnUpdDBSpkBook_clicked();

    void on_btnClearLocalSpkBook_clicked();


    void on_cbbNameSpkBook_currentIndexChanged(int i);
    void on_cbbGradeSpkBook_currentIndexChanged(int index);
    void on_cbbGenderSpkBook_currentIndexChanged(int index);
    void on_cbbBookSpkBook_currentIndexChanged(int index);


    void spkBookNameTextChanged(QString tc);
    void lwSpkBookItemSelected(QListWidgetItem * item);


    void on_btnCleanInactive_clicked();
    void on_btnScoreTnt_clicked();

    void on_btnScoreSpk_clicked();

    void on_btnCfgChange_clicked();

};

#endif // MAINWINDOW_H
