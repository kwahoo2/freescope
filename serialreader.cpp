#include "serialreader.h"

SerialReader::SerialReader(QObject *parent) :
    QThread(parent)
{
    stop = true;
    dataBuf.reserve(bufsize);
}

void SerialReader::setRun()
{
    if (!isRunning())
    {
        if (isStopped())
        {
            stop = false;
        }
        start(LowPriority);
        qDebug() << "Serial read thread started";
    }
}

void SerialReader::setStop()
{
    stop = true;
}

bool SerialReader::isStopped()
{
    return this->stop;
}

void SerialReader::run()
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    uint i = 0;

    while(!stop)
    {
        uint value = dummySerial();

        dataItem dItem;
        clock_gettime(CLOCK_MONOTONIC, &end);
        t_s = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1.0e9;
        dItem.readTime = t_s;
        dItem.readVal = value;

        dataBuf[i % bufsize] = dItem; //simple circular buffer implementation
        i++;

        //qDebug() << t_ns/1e9 << "  " << value;
    }

}

SerialReader::~SerialReader()
{
    stop = true;
}

SerialReader::dataItem SerialReader::readBufAt(int val)
{
    return dataBuf[val];
}

uint SerialReader::dummySerial()
{
    QThread::msleep(50);
    uint random = rand() % 1024;
    return random;
}
/*buffer emiter thread
*
*
*
*
*/

BufEmiter::BufEmiter(QObject *parent) :
    QThread(parent)
{
    mySerialReader = new SerialReader(this);
    stop = true;
}

void BufEmiter::setRun()
{
    if (!isRunning())
    {
        if (isStopped())
        {
            stop = false;
        }
        start(LowPriority);
        qDebug() << "Serial read thread started";
    }
}

void BufEmiter::setStop()
{
    stop = true;
}

bool BufEmiter::isStopped()
{
    return this->stop;
}

void BufEmiter::run()
{
    mySerialReader->setRun(); //thread reading serial

    int i = 0;
    SerialReader::dataItem data; //structure from inside class
    double oldTime = 0;
    while (!stop)
    {
        data = mySerialReader->readBufAt(i);
        if (oldTime <= data.readTime)
        {
            qDebug() << data.readTime << "   " << data.readVal << "  " << i;

            emit emitData(data.readTime,
                          data.readVal);

            oldTime = data.readTime;
        QThread::msleep(16); //60 fps update
        }
        i++;

        if (i > (mySerialReader->bufsize)) i = 0; //circular buffer
    }
}

BufEmiter::~BufEmiter()
{
    stop = true;
    mySerialReader->setStop();
}

