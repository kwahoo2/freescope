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
    triggerLevel = 0;
    fallingEdge = false;
    risingEdge = false;
    chIn = {0, 0, 0, 0,
          0, 0, 0, 0};
    chOut = {0, 0, 0, 0,
          0, 0, 0, 0};
    cellFormula = {"1", "1", "1", "1",
          "1", "1", "1", "1"};
    engine = new QScriptEngine;
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

}


void MainWindow::on_pushButton_2_clicked()
{
    getFormulas();

    if(!(myBufEmiter->isStarted()))
    {
    myBufEmiter->readBuffer();
    QObject::connect(myBufEmiter, SIGNAL(emitData(const vector <double>, //varaible time update
                                                     const vector<int>)),
                     this, SLOT(updateGraphsData(const vector<double>,
                                             const vector<int>)));
        ui->pushButton_2->setText(tr("Started"));
    }
    else
    {
        myBufEmiter->stopReadBuffer();
        QObject::disconnect(myBufEmiter, SIGNAL(emitData(const vector <double>,
                                                         const vector<int>)),
                         this, SLOT(updateGraphsData(const vector<double>,
                                                 const vector<int>)));
        ui->pushButton_2->setText(tr("Stopped"));
    }

}

void MainWindow::updateGraphsData(const vector<double> t,
                              const vector<int> val)
{
    double valNew = 0;
    double valOld = 0;
    bool isTriggered = false;
    for (int i = 0; i < 8; i++)
    {
        if ((myBufEmiter->activeCh) & (1 << i))
        {
            double valD = static_cast<double> (val[i]);
            chIn[i] = valD;

            iCell = new QTableWidgetItem;
            iCell->setText((QString::number(valD))); // fill the spreadsheet
            ui->tableWidget->setItem(i, 0, iCell);


        }
        //qDebug() << i << "  " << t[i] << "   " << valD << "  ";

    }
    updateSpreadSheet();
    for (int i = 0; i < 8; i++)
    {
        if (triggerEnabled && (triggerCh == i))
        {
            valNew = chOut[i];
            isTriggered = checkIfTriggered(valNew, valOld);
            valOld = valNew;
        }
        ui->plot->graph(i)->addData(t[i], chOut[i]);
        ui->plot->graph(i)->rescaleValueAxis();
        ui->plot->xAxis->setRange(t[i]+0.1, 3, Qt::AlignRight);
    }

    if (!triggerEnabled)
    {
        refreshGraphs();
    }
    else
    {
        if (isTriggered)
        {
            refreshGraphs();
        }
    }


}

void MainWindow::refreshGraphs()
{
    ui->plot->replot();
}

bool MainWindow::checkIfTriggered(const double valNew, const double valOld)
{
    if (risingEdge)
    {
        if ((triggerLevel < valNew) && (triggerLevel > valOld))
        {
            return true;
        }
    }
    if (fallingEdge)
    {
        if ((triggerLevel < valOld) && (triggerLevel > valNew))
        {
            return true;
        }
    }
    return false;
}
void MainWindow::updateSpreadSheet()
{
    /*for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        QString cellStr = ui->tableWidget->item(i, 0)->text();
        chIn[i] = cellStr.toDouble();
    }*/

    engine->globalObject().setProperty("CH0", chIn[0]);
    engine->globalObject().setProperty("CH1", chIn[1]);
    engine->globalObject().setProperty("CH2", chIn[2]);
    engine->globalObject().setProperty("CH3", chIn[3]);
    engine->globalObject().setProperty("CH4", chIn[4]);
    engine->globalObject().setProperty("CH5", chIn[5]);
    engine->globalObject().setProperty("CH6", chIn[6]);
    engine->globalObject().setProperty("CH7", chIn[7]);

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) //have to be started after full chIn[i] reading
    {
        chOut[i] = computeFormula(cellFormula[i]);
        //qDebug() << chOut[i];
    }
}

void MainWindow::refreshSpreadSheet()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
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
