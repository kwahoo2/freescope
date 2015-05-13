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
    void updateGraphs(const vector<double> t,
                      const vector<int> val);

    void on_checkBoxCh0_clicked(bool checked);

    void on_checkBoxCh1_clicked(bool checked);

    void on_checkBoxCh2_clicked(bool checked);

    void on_checkBoxCh3_clicked(bool checked);

    void on_checkBoxCh4_clicked(bool checked);

    void on_checkBoxCh5_clicked(bool checked);

    void on_checkBoxCh6_clicked(bool checked);

    void on_checkBoxCh7_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    BufEmiter *myBufEmiter;

};

#endif // MAINWINDOW_H
