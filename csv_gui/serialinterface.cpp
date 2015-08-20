#include "serialinterface.h"
#include <stdio.h>

SerialInterface::SerialInterface(QObject *parent) :
    QThread(parent)
{
    m_Port      = 0;
    m_barCode="";


    m_PortParams.m_SerialPort="COM1";
    m_PortParams.m_Baud     = BAUD9600 ;
    //m_PortParams.m_Baud     = BAUD256000;
    m_PortParams.m_Flow     = FLOW_OFF; // FLOW_XONXOFF;
    //m_PortParams.m_Flow     = FLOW_XONXOFF;
    m_PortParams.m_Parity   = PAR_NONE;
    m_PortParams.m_DataBits = DATA_8;
    m_PortParams.m_StopBits = STOP_1;

    m_timer = new QTimer(this);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

}


SerialInterface::~SerialInterface()
{
    quit ();
    wait (2000);
    if (isRunning ())
      terminate ();

}

void SerialInterface::run()
{
	
    printf( " SerialInterface::run \n" );

    QThread::exec();
    printf( " SerialInterface::exit \n" );


}

void SerialInterface::setConfig(SerialParams *params)
{
    printf( " SerialInterface::setConfig \n" );

   // m_PortParams = *params;


}


void SerialInterface::openSerial()
{
    printf( " SerialInterface::openSerial ( new and open) \n" );

    if (m_PortParams.m_SerialPort.isEmpty ()) {
        printf( "No COM port is set ??? \n");

        return;
    }

    if (m_Port) {
        //printf("port is already opened ????\n" );

        return;
    }

    m_Port = new QextSerialPort(QString ("\\\\.\\%1").arg(m_PortParams.m_SerialPort));

    m_Port->setBaudRate (m_PortParams.m_Baud);
    m_Port->setFlowControl (m_PortParams.m_Flow);
    m_Port->setParity (m_PortParams.m_Parity);
    m_Port->setDataBits (m_PortParams.m_DataBits);
    m_Port->setStopBits (m_PortParams.m_StopBits);
    m_Port->setTimeout(m_PortParams.m_Timeout);
    m_Port->setQueryMode(QextSerialPort::EventDriven);

    m_Port->open (QIODevice::ReadWrite);

    if (m_Port->isOpen ()) {

        connect (m_Port, SIGNAL(readyRead()), this, SLOT(receive()));

        m_Port->setBaudRate (m_PortParams.m_Baud);
        m_Port->setFlowControl (m_PortParams.m_Flow);
        m_Port->setParity (m_PortParams.m_Parity);
        m_Port->setDataBits (m_PortParams.m_DataBits);
        m_Port->setStopBits (m_PortParams.m_StopBits);
        m_Port->setTimeout(m_PortParams.m_Timeout);

    }
    else {
            printf( "Could Not Open The Serial Port ! \n" );

        delete m_Port;
        m_Port = 0;
        return;
    }
    printf( "Serial Interface : Serial Port Opened \n" );

    return;
}



void SerialInterface::closeSerial()
{
    printf("SerialInterface::closeSerial \n" );
    if (m_Port) {
        if (m_Port->isOpen ()) {
            printf("disconnect and close port  \n" );
            m_Port->flush();
            disconnect (m_Port, SIGNAL(readyRead()), this, SLOT(receive()));
            m_Port->close ();

        }
        delete m_Port;
        m_Port = 0;
    }

    return;
}

void SerialInterface::onTimeout()
{
    if ( m_barCode.length() > 0 )
    {
        emit receivedBarCode(m_barCode);
        m_barCode = "";
        m_timer->stop();
    }
}

void SerialInterface::receive()
{

    printf( "receive ..\n" );

    int bytes;

    char pktStart ;
    if(! m_timer->isActive() ){
        m_timer->start (1000);
    }

    bytes = m_Port->bytesAvailable();
    if( bytes <= 0 ) return;

    printf( "bytes:%d \n" , bytes);

    for( int i = 0; i< bytes; i++){

        if (-1 != m_Port->read ((char *)(&pktStart), 1)) {


           if ( pktStart == '\r' || pktStart == '\n' )
           {
               printf("find barcode:%s \n",qPrintable(m_barCode) );

               if ( m_barCode.length() > 0 )
               {
                   emit receivedBarCode(m_barCode);
                   m_barCode = "";
                   m_timer->stop();
               }
           }
           else
           {
               m_barCode.append(pktStart);
           }

           //printf("barcode:%s \n",qPrintable(m_barCode) );


        }
        else{
            printf("read error \n");
        }

    }

}



