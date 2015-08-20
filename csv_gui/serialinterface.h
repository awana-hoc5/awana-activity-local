#ifndef SERIALINTERFACE_H
#define SERIALINTERFACE_H

//#include <QObject>
#include <QThread>
#include <QTimer>

#include <qextserialenumerator.h>
#include <qextserialport.h>


typedef struct SerialParamsTAG {
    QString		m_SerialPort;
    BaudRateType	m_Baud;
    FlowType	m_Flow;
    ParityType	m_Parity;
    DataBitsType	m_DataBits;
    StopBitsType	m_StopBits;
    long		m_Timeout;
} SerialParams;


//class SerialInterface : public QObject
class SerialInterface : public QThread
{
    Q_OBJECT
public:
    explicit SerialInterface(QObject *parent = 0);
    virtual ~SerialInterface();


    bool isOpen () { if (m_Port == 0) return false; return m_Port->isOpen (); }



private:


    virtual void run();

    QTimer * m_timer;


    QextSerialPort  *m_Port;
    SerialParams    m_PortParams;

    QString m_barCode ;



signals:
    void receivedBarCode (QString barcode);


public slots:
    void onTimeout();

    void openSerial ();
    void closeSerial ();
    void setConfig(SerialParams *params);

    void receive();

};

#endif // SERIALINTERFACE_H
