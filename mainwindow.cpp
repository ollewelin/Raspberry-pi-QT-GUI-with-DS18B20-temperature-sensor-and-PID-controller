#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<pthread.h>
#include "tempsens.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{


    //****** Create a Temperature sensor thread ***************
    tempsens *tempThread_a;
    pthread_mutex_t *mut;
    mut = new pthread_mutex_t;
    pthread_mutex_init(mut, NULL);
    tempThread_a = new tempsens(mut);

    // Start the thread, send it the this pointer (points to this class
    // instance)
    tempThread_a->Start();

    //*********************************************************

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
