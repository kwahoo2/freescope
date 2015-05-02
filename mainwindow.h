#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include "qcustomplot.h"
#include "serialreader.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setupPlot();

private slots:
    void on_pushButton_2_clicked();
    void updateGraphs(double t,
                      int val);

private:
    Ui::MainWindow *ui;
    BufEmiter *myBufEmiter;

};

#endif // MAINWINDOW_H
