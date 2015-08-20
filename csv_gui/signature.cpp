#include "signature.h"


var canvas= null ;
var canvasW= null ;
 var bgCol = "rgb(200,200,200)";
var lnCol = "rgb(0,0,0)";

int lnTk = 2;
 int SIZE_SEC = 50;

Signature::Signature()
{

    //int cW = 480;
    //int cH = 100;

      m_ctx =null;
     m_ctxW =null;


    // m_xLoc[SIZE_SIGNATURE];
     //m_yLoc[SIZE_SIGNATURE];



     cnt = 0;
     m_wSt = 0;
     m_pTimer  = null;
     m_drwCnt= 0;

    //var onObj = null;

     m_start = 0;
     m_stop = 0;

     m_xOff = 0;
     m_yOff = 0;


     m_di = 0;


     m_secCnt = 0;
}


int Signature::init()
{


   //var obj = document.getElementById("pad");
   var obj = document.getElementById("canwr");

    do {
        m_xOff += obj.offsetLeft;
        m_yOff += obj.offsetTop;
    } while (obj = obj.offsetParent);

    clearSig();

 //   canvas=document.getElementById("can");
    canvasW=document.getElementById("canwr");

 //   m_ctx=canvas.getContext("2d");
    m_ctxW=canvasW.getContext("2d");

    m_ctxW.fillStyle = bgCol; //"rgb(192,192,192)";

   m_ctxW.fillRect(0, 0 , 480, 100);
  // m_ctxW.fillStyle = "rgb(255,0,0)";
   m_ctxW.fillStyle = lnCol;
   m_ctxW.strokeStyle  = lnCol;


   m_ctxW.lineWidth = lnTk;

    onObj =  document.getElementById('on');

   getSign();
   drawSig();

   drawDone();
  // alert( "done");


}



void Signature::getSign()
{
    var signBody="{!signBody}";
    var locList = signBody.split(";");

    m_cnt = locList.length;

    for ( i = 0; i < m_cnt ; i ++ )
    {
        var loc = locList[i].split("," );
        m_xLoc[i] = loc[0];
        m_yLoc[i] = loc[1];

    }

}


void Signature::drawSig( )
{
    m_secCnt  = 0;
    int state = 0;
    m_ctxW.beginPath();
    for( i = 0; i < m_cnt ; i++){

        if( m_xLoc[i] == -1 ) {
             state = 0;
             continue;
        }
        switch( state)
        {
        case 0:
            m_ctxW.moveTo( m_xLoc[i], m_yLoc[i]);
            state = 1;

            break;
        default:
             m_ctxW.lineTo( m_xLoc[i], m_yLoc[i]);

            break;

        }
   }
   m_ctxW.stroke();

}



void Signature::saveSig()
{
    //var formObj = document.getElementById('signature');
    var signStr='';
    for (i = 0; i < m_cnt-1 ; i ++ )
    {
        signStr = signStr + m_xLoc[i] + ',' + m_yLoc[i] + ';';

    }
    signStr = signStr + m_xLoc[m_cnt-1] + ',' + m_yLoc[m_cnt-1] ;

    //var encStr = encodeBase64(signStr);
    //var urlEncStr = urlEncode( encStr );

    //formObj.value=urlEncStr ;

}

void Signature::pointDown (event)
{
   if( m_cnt >= SIZE_SIGNATURE) {
        m_wSt=4;
        return;

    }

    switch(m_wSt)
    {

        case 0:
        case 3:
            m_ctxW.strokeStyle  = lnCol;
            m_ctxW.lineWidth = lnTk;
            m_ctxW.beginPath();
            m_xLoc[m_cnt]= event.pageX - m_xOff;
            m_yLoc[m_cnt]= event.pageY -m_yOff;
            m_cnt ++;

            m_wSt = 1;

        break;


    }

}

void Signature::drawLine()
{

    m_ctxW.moveTo( m_xLoc[m_start], m_yLoc[m_start]);
    var i = m_start;
    for (i = m_start+1; i <= m_stop; i++)
    {
        m_ctxW.lineTo( m_xLoc[i], m_yLoc[i]);

     }
     m_ctxW.stroke();
}

void Signature::pointUp( event)
{
    if( m_pTimer != null ){
          clearTimeout( m_pTimer );

      }
    switch(m_wSt){

    default:


      if( m_cnt >= SIZE_SIGNATURE ) return;
      m_xLoc[m_cnt]= event.pageX - m_xOff;
      m_yLoc[m_cnt]= event.pageY -m_yOff;
      m_cnt ++;

      if(( m_cnt - m_start) > 1 ){
        m_stop = m_cnt -1;
        if( m_stop > m_start ){
            drawLine( );
           //alert( "pointUp:" + cnt);
        }

      }
        m_xLoc[m_cnt]= -1 ;
        m_yLoc[m_cnt]= -1;
        m_cnt ++;
        m_wSt  = 3;
        m_start = m_cnt ;
      break;
   }
}
void Signature::pointMove(event)
{


    switch(m_wSt )
    {

        case 1:
         if( m_cnt >= SIZE_SIGNATURE ) {

               // onObj.style.m_display ="block";
                m_wSt  = 4;

            }
            else{
                m_wSt  = 2;
                m_xLoc[m_cnt]= event.offsetX ;
                m_yLoc[m_cnt]= event.offsetY ;
                m_cnt ++;
                m_pTimer = setTimeout(checkPos, 200);
            }

        break;
       case 2:
         if( m_cnt >= SIZE_SIGNATURE ) {

               // onObj.style.m_display ="block";
                m_wSt  = 4;

            }
            else{
                m_xLoc[m_cnt]= event.pageX -m_xOff;
                m_yLoc[m_cnt]= event.pageY -m_yOff;
                m_cnt ++;
            }

        break;
       case 3:
       case 4:


        break;

    }

}

void Signature::checkPos( )
{
    if( m_cnt >= SIZE_SIGNATURE ) {
        alert( "overflow");
        return;
    }
    //alert( "checkPos " + cnt);
    int curP  = m_cnt;
    for( i = m_drwCnt; i < curP  ; i++){

        if( m_xLoc[i] == -1 ) {
             state = 0;
             continue;
        }
        putPen( m_xLoc[i], m_yLoc[i]);
   }
   m_drwCnt =  curP  ;

 m_pTimer = setTimeout(checkPos, 200);

}



void Signature::drawDone()
{
    m_cnt = 0;
    m_wSt  = 0;
    // alert("done " + m_di);
   clearSig();
   state = 0;
   m_di = 0;
   m_drwCnt = 0;

    m_start = 0;
    m_stop = 0;
}


void Signature::drawSec( )
{

    for(int i = 0; i < SIZE_SEC ; i++){
        if( m_di >= m_cnt ) {
            setTimeout(drawDone, 100);
            return ;
        }
        if( m_xLoc[m_di] == -1 ) {
             state = 0;
             //alert("up " + m_di);
             m_di++;
             continue;
        }
        switch( state)
        {
            case 0:
            movePen( m_xLoc[m_di], m_yLoc[m_di]);
            state = 1;

            break;
            default:
              drawPen( m_xLoc[m_di], m_yLoc[m_di]);

            break;

        }
        m_di++;

   }
   m_pTimer = setTimeout(drawSec , 300);
}


void Signature::putAll( )
{

    for( i = 0; i < m_cnt; i++){

        if( m_xLoc[i] == -1 ) {
             state = 0;

            // m_di++;
             continue;
        }
        switch( state)
        {
            case 0:
            putPendV( m_xLoc[i], m_yLoc[i]);
            state = 1;

            break;
            default:
              putPendV( m_xLoc[i], m_yLoc[i]);

            break;

        }


   }
  // drawDone();
}

void Signature::putSec( )
{

    for( i = 0; i < SIZE_SEC ; i++){
        if( m_di >= m_cnt ) {
            setTimeout(drawDone, 100);
            return ;
        }
        if( m_xLoc[m_di] == -1 ) {
             state = 0;

             m_di++;
             continue;
        }
        switch( state)
        {
            case 0:
            putPen( m_xLoc[m_di], m_yLoc[m_di]);
            state = 1;

            break;
            default:
              putPen( m_xLoc[m_di], m_yLoc[m_di]);

            break;

        }
        m_di++;

   }
   m_pTimer = setTimeout(putSec, 300);
}

void Signature::putSig()
{
    //alert("put " + cnt);
    m_secCnt  = 0;
    m_ctx.fillStyle = "rgb(192,192,192)";
    m_ctx.fillRect(0, 0 , 240, 100);
    m_ctx.fillStyle = "rgb(255,0,0)";
    m_pTimer = setTimeout(putAll, 200);

}

void Signature::clearDraw()
{


    m_ctxW.fillStyle = "rgb(200,200,200)";

   m_ctxW.fillRect(0, 0 , 480, 100);
  // m_ctxW.fillStyle = "rgb(255,0,0)";


   m_ctxW.fillStyle = lnCol;
   m_ctxW.strokeStyle  = lnCol;
     drawDone();

}

void Signature::clearSig()
{
       for( i = 0; i < SIZE_SIGNATURE ; i++){

          m_xLoc[i] = -1;
          m_yLoc[i] = -1;

       }
}

void Signature::putPendV(int x, int y)
{

 m_ctx.fillRect (x-1, y-1, 3, 3);


}

void Signature::putPen(int x,int y){


 m_ctxW.fillRect (x, y, 2, 2);


}
/*
void Signature::putPen(x,y)
 {
    m_ctxW.fillRect (x-1, y-1, 2, 2);
 }
*/

