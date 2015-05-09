#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QSerialPort>
#include <QTimer>
#include <ctime>
#include <memory> //needed for RAII (unique_ptr)

using namespace std;

class SerialReader : public QObject
{
    Q_OBJECT
public:
    explicit SerialReader(QObject *parent = 0);
    static const long bufsize = 100000;  //big circular buffer size
    long eBufCounter; //earlier cycle buffer write pos
    struct dataItem
    {
        quint16 readVal;
        double readTime;
        qint8 readId;
    };

protected:

signals:

private:
    static const qint64 maxBytes = 90; //small buffer for serial size
    long dBufCounter;
    double t_s, t_sum;

    struct timespec start, end;
    QByteArray serialBuffer;
    quint16 dummySerial(); //returns a random 0-1024 number
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

protected:

signals:
    void emitData(double time,
               int value);

private:
    bool stop;
    ~BufEmiter();
    SerialReader *mySerialReader;
    SerialReader::dataItem data, data2; //structure from inside class
    QMutex mutex;
    QTimer *timer;
    struct timespec start, end;

public slots:
    void readBuffer();
    void stopReadBuffer();

private slots:
    SerialReader::dataItem findData(double t, qint8 id);
    void updateGraph();

};

#endif // SERIALREADER_H
