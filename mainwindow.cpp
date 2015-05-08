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

    ui->plot->addGraph(); // blue line
    ui->plot->graph(0)->setPen(QPen(Qt::blue));
    ui->plot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    ui->plot->graph(0)->setAntialiasedFill(false);

    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");

}


void MainWindow::on_pushButton_2_clicked()
{


    myBufEmiter->readBuffer();
    QObject::connect(myBufEmiter, SIGNAL(emitData(double,int)),
                     this, SLOT(updateGraphs(double,int)));

}

void MainWindow::updateGraphs(double t,
                              int val)
{
    double valD = static_cast<double> (val);

    ui->plot->graph(0)->addData(t, valD);
    ui->plot->graph(0)->rescaleValueAxis();
    ui->plot->xAxis->setRange(t+0.25, 5, Qt::AlignRight);
    ui->plot->replot();
}
