#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<pthread.h>
#include"tempsignals.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    printf("TEST\n");
    tempsignals *tempsobj;
    tempsobj = new tempsignals;

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
