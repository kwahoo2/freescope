#include "serialreader.h"

SerialReader::SerialReader()
{
    dataBuf.reserve(bufsize);
    serial = new QSerialPort(this);
}

void SerialReader::openSerial()
{

    serial->setBaudRate(57600);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setPortName("/dev/ttyUSB0");
    serial->open(QIODevice::ReadOnly);

    t_s = 0;
    t_sum = 0;
    dBufCounter = 0; //counter for the circular buffer
    eBufCounter = 0;

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
           mutex.lock();
           // qDebug() << "Serial read... " << serialBuffer.size() << serialBuffer;

            t_sum += t_s; //cumulative time for buffer data
            t_s = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1.0e9;
            clock_gettime(CLOCK_MONOTONIC, &start); //get starting time for a next read
            double dt = t_s / serialBuffer.size(); //time of a single byte read
            dataItem dItem;

            if ((quint8)serialBuffer.at(0) < 0x80) //minimal value of the first byte is 10000000, maximal of the second is 01111111
            {
                serialBuffer.remove(0, 1);
            }
            if (serialBuffer.size() % 2) //remove the last byte if buffer is even
            {
                serialBuffer.remove(serialBuffer.size(), 1);
            }

            long tmpBufCounter = dBufCounter; //avoid overwriting variable

            for (int i = 0; i < serialBuffer.size(); i += 2)
            {
                quint16 data;
                quint8 tmp = serialBuffer[i];
                quint8 tmp2 = serialBuffer[i+1];
                data = (tmp << 7 | tmp2); //move only for 7 places to fill bit 7  in the second byte
                qDebug() << "TMP " << tmp << "TMP2 " << tmp2 << "DATA " <<data;
                /*last 16 bits
                 * 15 0
                 * 14 1
                 * 13:11 id
                 * 9:8 data from the first byte
                 * 7:0 data from the second byte*/

                quint16 id = (data >> 11) & 0x7; // 13:11 id
                data = data & 0x3FF; //0-1023 value

                //qDebug() << "id: " << id << "value: " <<data;

                dItem.readTime = t_sum + (dt * i);
                dItem.readVal = data;
                dItem.readId = id;

                dataBuf[dBufCounter % bufsize] = dItem; //simple circular buffer implementation
                dBufCounter++;

            } 
            eBufCounter = (tmpBufCounter % bufsize);
            serialBuffer.clear();
            mutex.unlock();
        }

}

void SerialReader::closeSerial()
{
    QObject::disconnect(serial, SIGNAL(readyRead()),
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


bool SerialReader::isOpened()
{
    return serial->isOpen();
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
    serThread = new QThread;
    activeCh = 0xFF;
    addInterval = 16;
    mySerialReader = new SerialReader();
    mySerialReader->moveToThread(serThread);
    started = false;
}


void BufEmiter::readBuffer()
{
    mySerialReader->clearBuf();
    mySerialReader->openSerial(); //thread reading serial
    if (mySerialReader->isOpened())
    {
        started = true;
        serThread->start();
        updateGraph();
    }
    else
    {
        serThread->exit();
        started = false;
    }
}

void BufEmiter::updateGraph()
{
    SerialReader::dataItem data, dataold;
    dataold.readId = 0; dataold.readTime = 0.0; dataold.readVal = 0;
    //long iBuf = mySerialReader->eBufCounter; //get start position
    long iBuf = 0;

    while (!stop)
    {
        data = mySerialReader->readBufAt(iBuf);
                    qDebug() << data.readTime << " " <<iBuf;
        if (data.readTime > dataold.readTime)
        {
            int id = data.readId;
            double t = data.readTime;
            int val = data.readVal;
            if (activeCh & (1 << id)) emit emitData(id, t, val);

            QThread::usleep(1000);
        }
        else
        {
            iBuf++;
            QThread::usleep(200);
        }
        dataold = data;
        if (iBuf > (mySerialReader->bufsize)) iBuf = 0; //circular buffer
    }
}

void BufEmiter::stopReadBuffer()
{
    mySerialReader->closeSerial();
    started = false;
}

void BufEmiter::enableCh(const int val)
{
   activeCh |= (1 << val);
   qDebug() << "Channel " << val << "enabled";
}

void BufEmiter::disableCh(const int val)
{
   activeCh &= ~(1 << val);
   qDebug() << "Channel " << val << "disabled";
}

bool BufEmiter::isStarted()
{
    return started;
}

BufEmiter::~BufEmiter()
{
    mySerialReader->closeSerial();
    //delete mySerialReader;
}

