#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    myBufEmiter = new BufEmiter(this);
    ui->setupUi(this);
    setupPlot();
    triggerEnabled =  false;
    triggerCh = 0;
    triggerLevel = ui->triggerSpinBox->value();
    fallingEdge = false;
    risingEdge = false;
    valNew = 0;
    valOld = 0;
    minYRange = 0;
    maxYRange = 5;
    baseTime = 3000;
    chIn = {0, 0, 0, 0,
          0, 0, 0, 0};
    chOut = {0, 0, 0, 0,
          0, 0, 0, 0};
    cellFormula = {"1", "1", "1", "1",
          "1", "1", "1", "1"};
    engine = new QScriptEngine;

    for (int i = 0; i < 8; i++)
    {
        ui->triggerComboBox->addItem("Channel "+QString::number(i));
    }
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()),
                     this, SLOT(refreshGraphs()));
    QObject::connect(timer, SIGNAL(timeout()),
                     this, SLOT(refreshSpreadSheet()));

}

MainWindow::~MainWindow()
{
    delete engine;
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

    ui->plot->xAxis->setLabel("t");
    ui->plot->yAxis->setLabel("V");
    ui->plot->yAxis->setRange(minYRange, maxYRange);

}


void MainWindow::on_pushButton_2_clicked()
{
    for (int i = 0; i < 8; i++)
    {
        ui->plot->graph(1)->clearData();
    }
    getFormulas();
    int plotWidth = ui->plot->size().width();
    baseTime = ui->baseTimeSpinBox->value();
    qDebug() << (baseTime/plotWidth);

    if(!(myBufEmiter->isStarted()))
    {
        myBufEmiter->readBuffer();
        QObject::connect(myBufEmiter, SIGNAL(emitData(const int, //varaible time update
                                                      const double,
                                                      const int)),
                     this, SLOT(updateGraphsData(const int,
                                                 const double,
                                                 const int)));
        ui->pushButton_2->setText(tr("Started"));
    }
    else
    {
        myBufEmiter->stopReadBuffer();
        QObject::disconnect(myBufEmiter, SIGNAL(emitData(const int, //varaible time update
                                                      const double,
                                                      const int)),
                     this, SLOT(updateGraphsData(const int,
                                                 const double,
                                                 const int)));
        timer->stop();
        ui->pushButton_2->setText(tr("Stopped"));
    }

}

void MainWindow::updateGraphsData(const int idx,
                                  const double t,
                                  const int val)
{
    bool isTriggered = false;
    if ((myBufEmiter->activeCh) & (1 << idx))
    {
        double valD = static_cast<double> (val);
        chIn[idx] = valD;
        iCell = new QTableWidgetItem;
        iCell->setText((QString::number(valD))); // fill the spreadsheet
        ui->tableWidget->setItem(idx, 0, iCell);
    }
    QString chStr = "CH"+QString::number(idx);
    engine->globalObject().setProperty(chStr, chIn[0]);
    chOut[idx] = computeFormula(cellFormula[idx]);

    if (myBufEmiter->activeCh & (1 << idx))
    {
        if (triggerEnabled && (triggerCh == idx))
        {
            valNew = chOut[idx];
            //qDebug() << "trig" << valNew <<" " << valOld;
            isTriggered = checkIfTriggered(valNew, valOld);
            valOld = valNew;
        }
        ui->plot->graph(idx)->addData(t, chOut[idx]);
        ui->plot->xAxis->setRange(t, baseTime/1000, Qt::AlignRight);
        //ui->plot->graph(i)->rescaleValueAxis();
    }

    if (triggerEnabled)
    {
        if (isTriggered)
        {
            refreshGraphs();
            refreshSpreadSheet();
        }
    }
}

void MainWindow::refreshGraphs()
{
    ui->plot->replot();
}

bool MainWindow::checkIfTriggered(const double valNew, const double valOld)
{
    if (fallingEdge)
    {
        if ((triggerLevel < valOld) && (triggerLevel > valNew))
        {
            return true;
        }
    }
    if (risingEdge)
    {
        if ((triggerLevel < valNew) && (triggerLevel > valOld))
        {
            return true;
        }
    }
    return false;
}

void MainWindow::refreshSpreadSheet()
{
    for (int i = 0; i < 8; i++)
    {
        iCell = new QTableWidgetItem;
        iCell->setText((QString::number(chOut[i])));
        ui->tableWidget->setItem(i, 2, iCell);
    }
}
void MainWindow::getFormulas()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) //have to be started after full chIn[i] reading
    {
        cellFormula[i] = ui->tableWidget->item(i, 1)->text();
    }
}

double MainWindow::computeFormula(const QString str)
{
    double val = engine->evaluate(str).toNumber();
    if (!std::isnan(val))
    {
        return val;
    }
    else
    {
        return 0;
    }
}
void MainWindow::on_checkBoxCh0_clicked(bool checked)
{
    if (checked)
    {
        myBufEmiter->enableCh(0);
    }
    else
    {
        myBufEmiter->disableCh(0);
    }
}

void MainWindow::on_checkBoxCh1_clicked(bool checked)
{
    if (checked)
    {
        myBufEmiter->enableCh(1);
    }
    else
    {
        myBufEmiter->disableCh(1);
    }
}

void MainWindow::on_checkBoxCh2_clicked(bool checked)
{
    if (checked)
    {
        myBufEmiter->enableCh(2);
    }
    else
    {
        myBufEmiter->disableCh(2);
    }
}

void MainWindow::on_checkBoxCh3_clicked(bool checked)
{
    if (checked)
    {
        myBufEmiter->enableCh(3);
    }
    else
    {
        myBufEmiter->disableCh(3);
    }
}

void MainWindow::on_checkBoxCh4_clicked(bool checked)
{
    if (checked)
    {
        myBufEmiter->enableCh(4);
    }
    else
    {
        myBufEmiter->disableCh(4);
    }
}

void MainWindow::on_checkBoxCh5_clicked(bool checked)
{
    if (checked)
    {
        myBufEmiter->enableCh(5);
    }
    else
    {
        myBufEmiter->disableCh(5);
    }
}

void MainWindow::on_checkBoxCh6_clicked(bool checked)
{
    if (checked)
    {
        myBufEmiter->enableCh(6);
    }
    else
    {
        myBufEmiter->disableCh(6);
    }
}

void MainWindow::on_checkBoxCh7_clicked(bool checked)
{
    if (checked)
    {
        myBufEmiter->enableCh(7);
    }
    else
    {
        myBufEmiter->disableCh(7);
    }
}

void MainWindow::on_maxSpinBox_valueChanged(double arg1)
{
    maxYRange = arg1;
    ui->plot->yAxis->setRange(minYRange, maxYRange);
    ui->plot->replot();
}

void MainWindow::on_minSpinBox_valueChanged(double arg1)
{
    minYRange = arg1;
    ui->plot->yAxis->setRange(minYRange, maxYRange);
    ui->plot->replot();
}

void MainWindow::on_triggerCheckBox_clicked(bool checked)
{
    triggerEnabled = checked;
    if (checked)
    {
        timer->stop();
    }
    else
    {
        timer->start(16);
    }
}

void MainWindow::on_risingCheckBox_clicked(bool checked)
{
    risingEdge = checked;
}

void MainWindow::on_fallingCheckBox_clicked(bool checked)
{
    fallingEdge = checked;
}

void MainWindow::on_triggerComboBox_currentIndexChanged(int index)
{
    qDebug() << index;
    triggerCh = index;
}

void MainWindow::on_triggerSpinBox_valueChanged(double arg1)
{
    triggerLevel = arg1;
}

void MainWindow::on_baseTimeSpinBox_valueChanged(double arg1)
{
    baseTime = arg1;
}
