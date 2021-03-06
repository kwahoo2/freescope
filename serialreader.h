#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QSerialPort>
#include <QTimer>
#include <ctime>
#include <QSerialPortInfo>
//#include <memory> //needed for RAII (unique_ptr)

using namespace std;

class SerialReader : public QObject
{
    Q_OBJECT
public:
    explicit SerialReader(QObject *parent = 0);
    static const long bufsize = 100000;  //big circular buffer size
    long eBufCounter; //earlier cycle buffer write pos
    bool isOpened();
    void sendByte(const qint8);
    struct dataItem
    {
        quint16 readVal;
        double readTime;
        qint8 readId;
    };
    QList <QSerialPortInfo> ports;

protected:

signals:

private:
    static const qint64 maxBytes = 90; //small buffer for serial size
    long dBufCounter;
    double t_s, t_sum;

    struct timespec start, end;
    QByteArray serialBuffer;
    QSerialPort *serial;
    QMutex mutex;
    ~SerialReader();

    vector<dataItem> dataBuf;

private slots:
    void readSerial();

public slots:
    void openSerial();
    void closeSerial();
    dataItem readBufAt(long val);
    void clearBuf();
    void refreshPorts();
    void setPort(const int val);

};
/*buffer emiter thread
*
*
*
*/
class BufEmiter : public QObject
{
    Q_OBJECT
public:
    explicit BufEmiter(QObject *parent = 0);
    bool isStarted();
    qint8 activeCh; //binary number representing active channels, used becasue future uC communication
    SerialReader *mySerialReader;

protected:

signals:
    void emitData(const vector<double> time,
               const vector<int> value);

private:
    bool stop;
    bool started;
    int addInterval;
    ~BufEmiter();
    SerialReader::dataItem data, data2; //structure from inside class
    QMutex mutex;
    QTimer *timer;
    struct timespec start, end;

public slots:
    void readBuffer();
    void stopReadBuffer();
    void enableCh(const int val);
    void disableCh(const int val);
    void setAddInterval(const double interval);

private slots:
    SerialReader::dataItem findData(const double t, const qint8 id);
    void updateGraph();

};

#endif // SERIALREADER_H
