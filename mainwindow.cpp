#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<pthread.h>
#include"tempsignals.h"
#include"controller.h"
#include<QVector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    printf("Create a temperature sensor QT object running on the QT thread side\n");
    tempsignals *tempsobj;//tempsignals contain the QT thread side of the temperature signals.
    tempsobj = new tempsignals;//This class will start the separate tempsens thread reading the DS18B20 sensors and copy over that data to this
    controller *controlobj;
    controlobj = new controller;
   // ui->lineEdit_T1->setValue(0.0f);

    connect(controlobj, SIGNAL(controllertick(void)), tempsobj, SLOT(gettemperature(void)));
    connect(tempsobj, SIGNAL(Temperature(QVector<float>)), this, SLOT(temperatures(QVector<float>)));
    connect(tempsobj, SIGNAL(Rom_vect(QVector<QString>)), this, SLOT(temp_id(QVector<QString>)));
    ui->setupUi(this);

}
void MainWindow::temperatures(QVector<float> tempvector)
{
   for(int i=0;i<tempvector.size();i++)
   {
       double temperat_l = (double)tempvector[i];
       switch (i) {
       case 0:
        ui->lineEdit_T1->setText(QString::number(temperat_l));
       break;
       case 1:
        ui->lineEdit_T2->setText(QString::number(temperat_l));
       break;
       case 2:
        ui->lineEdit_T3->setText(QString::number(temperat_l));
       break;
       case 3:
        ui->lineEdit_T4->setText(QString::number(temperat_l));
       break;
       case 4:
        ui->lineEdit_T5->setText(QString::number(temperat_l));
       break;
       case 5:
        ui->lineEdit_T6->setText(QString::number(temperat_l));
       break;
       case 6:
        ui->lineEdit_T7->setText(QString::number(temperat_l));
       break;
       case 7:
        ui->lineEdit_T8->setText(QString::number(temperat_l));
       break;
       case 8:
        ui->lineEdit_T9->setText(QString::number(temperat_l));
       break;
       case 9:
        ui->lineEdit_T10->setText(QString::number(temperat_l));
       break;

        }
   }

}

void MainWindow::temp_id(QVector<QString> temp_sens_id)
{
   for(int i=0;i<temp_sens_id.size();i++)
   {

       switch (i) {
       case 0:
        ui->lineEdit_T_ID1->setText(temp_sens_id[i]);
       break;
       case 1:
        ui->lineEdit_T_ID2->setText(temp_sens_id[i]);
       break;
       case 2:
        ui->lineEdit_T_ID3->setText(temp_sens_id[i]);
       break;
       case 3:
        ui->lineEdit_T_ID4->setText(temp_sens_id[i]);
       break;
       case 4:
        ui->lineEdit_T_ID5->setText(temp_sens_id[i]);
       break;
       case 5:
        ui->lineEdit_T_ID6->setText(temp_sens_id[i]);
       break;
       case 6:
        ui->lineEdit_T_ID7->setText(temp_sens_id[i]);
       break;
       case 7:
        ui->lineEdit_T_ID8->setText(temp_sens_id[i]);
       break;
       case 8:
        ui->lineEdit_T_ID9->setText(temp_sens_id[i]);
       break;
       case 9:
        ui->lineEdit_T_ID10->setText(temp_sens_id[i]);
       break;

        }
   }

}

MainWindow::~MainWindow()
{
    delete ui;
}
