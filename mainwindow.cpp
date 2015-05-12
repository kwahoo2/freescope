#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    myBufEmiter = new BufEmiter(this);
    ui->setupUi(this);
    setupPlot();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::setupPlot()
{


    //ui->plot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));//fill

    //ui->plot->graph(0)->setAntialiasedFill(false);
    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(Qt::blue));
    ui->plot->addGraph();
    ui->plot->graph(1)->setPen(QPen(Qt::red));
    ui->plot->addGraph();
    ui->plot->graph(2)->setPen(QPen(Qt::green));
    ui->plot->addGraph();
    ui->plot->graph(3)->setPen(QPen(Qt::yellow));
    ui->plot->addGraph();
    ui->plot->graph(4)->setPen(QPen(Qt::cyan));
    ui->plot->addGraph();
    ui->plot->graph(5)->setPen(QPen(Qt::black));
    ui->plot->addGraph();
    ui->plot->graph(6)->setPen(QPen(Qt::magenta));
    ui->plot->addGraph();
    ui->plot->graph(7)->setPen(QPen(Qt::darkRed));

    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");

}


void MainWindow::on_pushButton_2_clicked()
{

    if(!(myBufEmiter->isStarted()))
    {
    myBufEmiter->readBuffer();
    QObject::connect(myBufEmiter, SIGNAL(emitData(const vector <double>,
                                                     const vector<int>)),
                     this, SLOT(updateGraphs(const vector<double>,
                                             const vector<int>)));
        if (myBufEmiter->isStarted()) ui->pushButton_2->setText(tr("Started"));
    }
    else
    {
        myBufEmiter->stopReadBuffer();
        QObject::disconnect(myBufEmiter, SIGNAL(emitData(const vector <double>,
                                                         const vector<int>)),
                         this, SLOT(updateGraphs(const vector<double>,
                                                 const vector<int>)));
        ui->pushButton_2->setText(tr("Stopped"));
    }

}

void MainWindow::updateGraphs(const vector<double> t,
                              const vector<int> val)
{
    for (int i = 1; i < 6; i++) //set 1-6 for manual testing
    {
        double valD = static_cast<double> (val[i]);
        //qDebug() << val[i];
        ui->plot->graph(i)->addData(t[i], valD);
        ui->plot->graph(i)->rescaleValueAxis();

    }

    ui->plot->xAxis->setRange(t[2]+0.25, 3, Qt::AlignRight); //use PC1 signal as a base for timing
    ui->plot->replot();
}
