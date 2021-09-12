#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<pthread.h>
#include"tempsignals.h"
#include"heatpump.h"
#include"controller.h"
#include<QVector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)



{

    printf("Create a temperature sensor QT object running on the QT thread side\n");
    tempsignals *tempsobj;//tempsignals contain the QT thread side of the temperature signals.
    tempsobj = new tempsignals;//This class will start the separate tempsens thread reading the DS18B20 sensors and copy over that data to this
    heatpump *heatpobj;
    heatpobj = new heatpump;//This class will start the separate heatpump_socket_client thread writing/reading to a socket to a python script talking with tinytuya tuya device
    int nrofheatpsignals = heatpobj->nrofheatpsignals;
    heatpump_send.clear();
    heatpump_reply.clear();
    for(int i=0;i<nrofheatpsignals;i++){
        int defval=0;
        if(i==1){
            defval=20;
        }
        heatpump_send.push_back(defval);
        heatpump_reply.push_back(0);
    }
    last_selected_hotwater_temp = 55;
    last_selected_hot_temp = 20;

    controller *controlobj;
    controlobj = new controller;
   // ui->lineEdit_T1->setValue(0.0f);

    connect(controlobj, SIGNAL(controllertick(void)), tempsobj, SLOT(gettemperature(void)));
    connect(tempsobj, SIGNAL(Temperature(QVector<float>)), this, SLOT(temperatures(QVector<float>)));
    connect(tempsobj, SIGNAL(Rom_vect(QVector<QString>)), this, SLOT(temp_id(QVector<QString>)));

    ui->setupUi(this);
    connect(this, SIGNAL(setheatpump(QVector<int>)), heatpobj, SLOT(setheatpump(QVector<int>)));
    connect(this, SIGNAL(test(void)), heatpobj, SLOT(test(void)));

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

void MainWindow::on_checkBox_on_off_clicked(bool checked)
{
    ui->checkBox_auto->setChecked(false);
    ui->checkBox_manual->setChecked(true);
    if(checked == true){
        heatpump_send[0] = 2;//2=Turn ON heatpump device
    }
    else{
        heatpump_send[0] = 1;//
    }
    emit setheatpump(heatpump_send);
    printf("heatpump_send[0] = %d\n", heatpump_send[0]);
}

void MainWindow::on_spinBox_man_temp_hp_valueChanged(int arg1)
{
    ui->checkBox_auto->setChecked(false);
    ui->checkBox_manual->setChecked(true);
    ui->checkBox_heater_mode->setChecked(true);
    ui->checkBox_hotwater_mode->setChecked(false);
    ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    heatpump_send[1] = arg1;//Manual temperature set heatpump
    last_selected_hot_temp = arg1;
    emit setheatpump(heatpump_send);
    printf("heatpump_send[1] = %d\n", heatpump_send[1]);
}


void MainWindow::on_checkBox_hotwater_mode_clicked(bool checked)
{
    if(checked==true){
        heatpump_send[0] = 4;//4=hotwater mode (Only hotwater mode selected)
        heatpump_send[1] = last_selected_hotwater_temp;
        ui->checkBox_auto->setChecked(false);
        ui->checkBox_manual->setChecked(true);
        ui->checkBox_heater_mode->setChecked(false);
        //ui->checkBox_hotwater_mode->setChecked(true);
        ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    }
    emit setheatpump(heatpump_send);
}
//The command and reply data is in a form of 10 array c_uint32 data send/recive
//array index [0] = "command"
//    0=Do nothing
//    1=Turn OFF heatpump device
//    2=Turn ON heatpump device
//    3=hot_hotwater mode (heater + hotwater mode selected)
//    4=hotwater mode (Only hotwater mode selected)
//    5=hot mode (Only heater mode)
//    6=set_temp
//    7=only_read_status

//    10=cool mode, not implemented yet
//    11=cool_hotwater mode, not implemented yet
//array index [1] = setpoint temperature
//array index [2] = readback ON/OFF
//array index [3] = readback actual temperature
//array index [4] = readback setpoint temperature
//array index [5] = readback mode, mode readback code same as array index [0] = "command"

void MainWindow::on_checkBox_heater_mode_clicked(bool checked)
{
    if(checked==true){
        heatpump_send[0] = 5;//5=hot mode (Only heater mode)
        heatpump_send[1] = last_selected_hot_temp;
        ui->checkBox_auto->setChecked(false);
        ui->checkBox_manual->setChecked(true);
        //ui->checkBox_heater_mode->setChecked(true);
        ui->checkBox_hotwater_mode->setChecked(false);
        ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    }
    emit setheatpump(heatpump_send);
}

void MainWindow::on_spinBox_manual_hotwater_valueChanged(int arg1)
{
    ui->checkBox_auto->setChecked(false);
    ui->checkBox_manual->setChecked(true);
    ui->checkBox_heater_mode->setChecked(false);
    ui->checkBox_hotwater_mode->setChecked(true);
    ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    heatpump_send[1] = arg1;//Manual temperature set heatpump
    last_selected_hotwater_temp = arg1;
    emit setheatpump(heatpump_send);
    printf("heatpump_send[1] = %d\n", heatpump_send[1]);
}

void MainWindow::on_checkBox_hotwater_and_heater_mode_clicked(bool checked)
{
    if(checked==true){
        heatpump_send[0] = 3;//3=hot_hotwater mode (heater + hotwater mode selected)
        ui->checkBox_auto->setChecked(false);
        ui->checkBox_manual->setChecked(true);
        ui->checkBox_heater_mode->setChecked(false);
        ui->checkBox_hotwater_mode->setChecked(false);
        //ui->checkBox_hotwater_and_heater_mode->setChecked(true);
    }
    emit setheatpump(heatpump_send);

}

void MainWindow::on_checkBox_manual_clicked(bool checked)
{
    if(checked==true){
        ui->checkBox_auto->setChecked(false);
        ui->checkBox_on_off->setChecked(true);
        //ui->checkBox_manual->setChecked(true);
    }
}

void MainWindow::on_checkBox_auto_clicked(bool checked)
{
    if(checked==true){
        //ui->checkBox_auto->setChecked(true);
        ui->checkBox_manual->setChecked(false);
    }

}
