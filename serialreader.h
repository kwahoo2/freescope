#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <ctime>

using namespace std;

class SerialReader : public QThread
{
    Q_OBJECT
public:
    explicit SerialReader(QObject *parent = 0);
    static const int bufsize = 500;
    struct dataItem
    {
        uint readVal;
        double readTime;
    };

protected:
    void run();

signals:

private:
    double t_s;
    bool stop;
    bool isStopped();
    uint dummySerial(); //returns a random 0-1024 number
    ~SerialReader();

    vector<dataItem> dataBuf;


public slots:
    void setRun();
    void setStop();
    dataItem readBufAt(int val);

};
/*buffer emiter thread
*
*
*
*/
class BufEmiter : public QThread
{
    Q_OBJECT
public:
    explicit BufEmiter(QObject *parent = 0);
    bool isStopped();
    void setRun();
    void setStop();

protected:
    void run();

signals:
    void emitData(double time,
               int value);

private:
    bool stop;
    ~BufEmiter();
    SerialReader *mySerialReader;

public slots:
};

#endif // SERIALREADER_H
