#include "serialreader.h"

SerialReader::SerialReader(QObject *parent) :
    QThread(parent)
{
    stop = true;
    dataBuf.reserve(bufsize);
    serial = new QSerialPort(this);
    serial->setBaudRate(57600);
    serial->setPortName("/dev/ttyUSB0");
    serial->open(QIODevice::ReadOnly);

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
    clock_gettime(CLOCK_MONOTONIC, &start); //TODO: x (time) axis issues
    uint i = 0;

    while(!stop)
    {
        //
        //quint16 value = dummySerial();
        //
        //qDebug() << serial->bytesAvailable();
        if (serial->bytesAvailable())
        {
            quint16 value = readSerial();
            quint16 id = (value >> 11) & 0x7; //* 13:11 id
            value = value & 0x3FF;

            //qDebug() << "id: " << id << "value: " <<value;

            dataItem dItem;
            clock_gettime(CLOCK_MONOTONIC, &end);
            t_s = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1.0e9;
            dItem.readTime = t_s;
            dItem.readVal = value;

            if (id == 2) //show single channel
            {
                dataBuf[i % bufsize] = dItem; //simple circular buffer implementation
                i++;
            }
        }

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

void SerialReader::clearBuf()
{
    for (int n = 0; n < bufsize; n++)
    {
        dataBuf[n].readTime = 0;
        dataBuf[n].readVal = 0;
    }
    qDebug() << "Buffer cleared";
}


quint16 SerialReader::dummySerial()
{
    QThread::msleep(5);
    quint16 random = rand() % 1024;
    return random;
}
quint16 SerialReader::readSerial()
{
        char c;
        quint8 tmp = 0;
        quint8 tmp2 = 0;
        quint16 data;
        serial->getChar(&c);
        tmp = (quint8)c;
        if (tmp >= 0x80) //minimalna wartosc pierszego bajtu 10000000, maksymalna drugiego 01111111
        {
            serial->getChar(&c);
            tmp2 = (quint8)c;
        }
        else
        {
            serial->getChar(&c);
            tmp = (quint8)c;
            serial->getChar(&c);
            tmp2 = (quint8)c;
        }

        data = (tmp << 7 | tmp2); //przesuniecie tylko o 7 zeby wypelic pusty bit 7 w drugim bajcie

        /*ostatnie 16bitów
         * 15 0
         * 14 1
         * 13:11 id
         * 9:8 dane z pierwszego bajtu
         * 7:0 dane z drugiego bajtu*/

        QThread::usleep(200); //hack

        return data;
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
        qDebug() << "Serial emiter thread started";
    }
}

void BufEmiter::setStop()
{
    stop = true;
    mySerialReader->setStop();
}

bool BufEmiter::isStopped()
{
    return this->stop;
}

void BufEmiter::run()
{
    mySerialReader->clearBuf();
    mySerialReader->setRun(); //thread reading serial
    int i = 0;
    double oldTime;
    //std::unique_ptr<double> oldT(new double);
    oldTime = 0;

    while (!stop)
    {
        data = mySerialReader->readBufAt(i);
        //qDebug() << data.readTime << "...old" << *oldT;
        if (oldTime < data.readTime)
        {
            if ((i % 16) == 0) //60 fps update
            {
                qDebug() << data.readTime << "   " << data.readVal << "  " << i;
                emit emitData(data.readTime,
                          data.readVal);
            }
            oldTime = data.readTime;
            QThread::usleep(200);
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

