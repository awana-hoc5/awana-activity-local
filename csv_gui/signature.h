#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <QTimer>

#define SIZE_SIGNATURE 2000

class Signature
{
public:
    Signature();

private:

    //int cW = 480;
    //int cH = 100;

    int m_ctx ;
    int m_m_ctxW ;


    int m_xLoc[SIZE_SIGNATURE];
    int m_yLoc[SIZE_SIGNATURE];

    //var SIZE_SIGNATURE = 2000;

    int cnt ;
    int m_wSt ;
    QTimer * m_pTimer ;
    int m_drwCnt ;

    //var onObj = null;

    int m_start  ;
    int m_stop  ;

    int m_xOff  ;
    int m_yOff  ;


    int m_di  ;


    int m_secCnt  ;
};

#endif // SIGNATURE_H
