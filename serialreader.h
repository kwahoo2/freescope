#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QSerialPort>
#include <QTimer>
#include <ctime>
//#include <memory> //needed for RAII (unique_ptr)

using namespace std;

class SerialReader : public QObject
{
    Q_OBJECT
public:
    explicit SerialReader();
    static const long bufsize = 100000;  //big circular buffer size
    long eBufCounter; //earlier cycle buffer write pos
    bool isOpened();
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
    bool isStarted();
    qint8 activeCh; //binary number representing active channels, used becasue future uC communication

protected:

signals:
    void emitData(const int id,
                  const double time,
                  const int value);

private:
    bool stop;
    bool started;
    int addInterval;
    ~BufEmiter();
    SerialReader *mySerialReader;
    QThread *serThread;

public slots:
    void readBuffer();
    void stopReadBuffer();
    void enableCh(const int val);
    void disableCh(const int val);

private slots:
    void updateGraph();

};

#endif // SERIALREADER_H
