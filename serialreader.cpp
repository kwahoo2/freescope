#include "serialreader.h"

SerialReader::SerialReader(QObject *parent) :
    QObject(parent)
{
    dataBuf.reserve(bufsize);
}

void SerialReader::openSerial()
{
    serial = new QSerialPort(this);
    serial->setBaudRate(57600);
    serial->setPortName("/dev/ttyUSB0");
    serial->open(QIODevice::ReadOnly);

    t_s = 0;
    t_sum = 0;
    dBufCounter = 0; //counter for the circular buffer

    //serial->setReadBufferSize(maxBytes);
    serial->clear();
    QObject::connect(serial, SIGNAL(readyRead()),
                     this, SLOT(readSerial()));


    clock_gettime(CLOCK_MONOTONIC, &start);
}

void SerialReader::readSerial()
{

        serialBuffer += serial->readAll();
        clock_gettime(CLOCK_MONOTONIC, &end);

        if (serialBuffer.size() >= maxBytes)
        {
           // qDebug() << "Serial read... " << serialBuffer.size() << serialBuffer;

            t_sum += t_s; //cumulative time for buffer data
            t_s = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1.0e9;
            clock_gettime(CLOCK_MONOTONIC, &start); //get starting time for a next read
            double dt = t_s / serialBuffer.size(); //time of a single byte read

            if ((quint8)serialBuffer.at(0) < 0x80) //minimal value of the first byte is 10000000, maximal of the second is 01111111
            {
                serialBuffer.remove(0, 1);
            }
            if (serialBuffer.size() % 2) //remove the last byte if buffer is even
            {
                serialBuffer.remove(serialBuffer.size(), 1);
            }

            for (int i = 0; i < serialBuffer.size(); i += 2)
            {
                quint16 data;
                quint16 tmp = serialBuffer[i];
                quint16 tmp2 = serialBuffer[i+1];
                data = (tmp << 7 | tmp2); //move only for 7 places to fill bit 7  in the second byte
                /*last 16 bits
                 * 15 0
                 * 14 1
                 * 13:11 id
                 * 9:8 data from the first byte
                 * 7:0 data from the second byte*/

                quint16 id = (data >> 11) & 0x7; // 13:11 id
                data = data & 0x3FF; //0-1023 value

                //qDebug() << "id: " << id << "value: " <<data;

                dataItem dItem;

                dItem.readTime = t_sum + (dt * i);
                dItem.readVal = data;
                dItem.readId = id;

                dataBuf[dBufCounter % bufsize] = dItem; //simple circular buffer implementation
                dBufCounter++;

            }
            serialBuffer.clear();
        }

}

void SerialReader::closeSerial()
{
    QObject::disconnect(serial, SIGNAL(bytesWritten(minBytes)),
                     this, SLOT(readSerial()));
    serial->close();
}

SerialReader::~SerialReader()
{
    serial->close();
}

SerialReader::dataItem SerialReader::readBufAt(long val)
{
    return dataBuf[val];
}

void SerialReader::clearBuf()
{
    for (int n = 0; n < bufsize; n++)
    {
        dataBuf[n].readTime = 0;
        dataBuf[n].readVal = 0;
        dataBuf[n].readId = 0;
    }
    qDebug() << "Buffer cleared";
}


quint16 SerialReader::dummySerial()
{
    QThread::msleep(5);
    quint16 random = rand() % 1024;
    return random;
}

/*buffer emiter thread
*
*
*
*
*/

BufEmiter::BufEmiter(QObject *parent) :
    QObject(parent)
{
    mySerialReader = new SerialReader(this);
    timer = new QTimer(this);
    iBuf = 0;
}


void BufEmiter::readBuffer()
{
    mySerialReader->clearBuf();
    mySerialReader->openSerial(); //thread reading serial
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGraph()));
    timer->start(16); //60 fps update

    clock_gettime(CLOCK_MONOTONIC, &start);
}

void BufEmiter::updateGraph()
{
    clock_gettime(CLOCK_MONOTONIC, &end);
    double actTime = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1.0e9;
    data = findData(actTime, 2);

    qDebug() << data.readTime << "   " << data.readVal << "  ";
    emit emitData(data.readTime,
              data.readVal);
}

SerialReader::dataItem BufEmiter::findData(double t, qint8 id)
{

    SerialReader::dataItem tmpdata;
    double error = 0.1; //maximal alloved error time

    while (1)
    {
        tmpdata = mySerialReader->readBufAt(iBuf);

        if (id == tmpdata.readId)
        {
            //qDebug() << "t " << t << "readtime " <<tmpdata.readTime << "iBuf " << iBuf;
            if ((t > tmpdata.readTime) && (t< (tmpdata.readTime + error)))
            {
                return tmpdata;
            }
        }
        iBuf++;
        if (iBuf > (mySerialReader->bufsize)) iBuf = 0; //circular buffer
    }
}

void BufEmiter::stopReadBuffer()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(updateGraph()));
    timer->stop();
}

BufEmiter::~BufEmiter()
{

}

