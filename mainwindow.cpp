#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<pthread.h>
#include"tempsignals.h"
#include"heatpump.h"
#include"controller.h"
#include<QVector>
#include<QPixmap>
#include <QFileDialog>

#define NR_TEMP_SENSOR_GUI 10

//Communication to heatpump_server.py
//The command and reply data is in a form of 10 array c_uint32 data send/recive
//array index [0] = "command"
//    0=do nothig
//    1=Turn OFF heatpump device
//    2=Turn ON heatpump device
//    3=hot_hotwater mode (heater + hotwater mode selected)
//    4=hotwater mode (Only hotwater mode selected)
//    5=hot mode (Only heater mode)
//    6=set_temp
//    7=only status read
//    10=cool mode, not implemented yet
//    11=cool_hotwater mode, not implemented yet
//array index [1] = setpoint temperature
//array index [2] = readback ON/OFF
//array index [3] = readback actual temperature
//array index [4] = readback setpoint temperature
//array index [5] = readback mode, mode readback code same as array index [0] = "command"
//array index [6] = u'5' readback, "u'HotWater'" = 444, "u'Hot'" = 555, "u'Hot_HotWater'" = 333
//array index [7] = u'4' readback,
//array index [8] = u'1' readback,
//array index [9] = u'Error',
#define CMD_INDEX_0 0
#define CMD_DO_NOTHING 0
#define CMD_TURN_OFF 1
#define CMD_TURN_ON 2
#define CMD_hot_hotwater 3
#define CMD_hotwater 4
#define CMD_hot 5
#define CMD_SET_TEMP 6
#define CMD_STATUS_READ 7
#define CMD_cool 10
#define CMD_cool_hotwater 11
#define SET_INDEX_1_SETP_TEMP 1
#define REPLY_INDEX_2_ON_OFF 2
#define REPLY_INDEX_3_ACTUAL_TEMP 3
#define REPLY_INDEX_4_SETP_TEMP 4
#define REPLY_INDEX_5_MODE 5
#define REPLY_INDEX_6_u5 6
#define REPLY_INDEX_7_u4 7
#define REPLY_INDEX_8_u1 8
#define REPLY_INDEX_9_uError 9

#define REPLY_I6_HotWater_444 444
#define REPLY_I6_Hot_555 555
#define REPLY_I6_Hot_HotWater_333 333
#define REPLY_I7_hot_55 55
#define REPLY_I7_hot_hotwater_33 33
#define REPLY_I7_hotwater_44 44
#define REPLY_I8_22_ON 22
#define REPLY_I8_11_OFF 11
#define REPLY_I9_NETWORK_OK 0
#define REPLY_I9_NETWORK_ERR 1001



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)



{
//Store GUI settings
    switch_mode = false;
    temp_profile = 0.0;
    outside_temp = 0.0;
    auto_init_done = false;
    ui->setupUi(this);
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, "./");
    WorkSettingsPath = "WorkSettingFolder";//WorkSettingsPath path
    WorkSettingsFile = "WorkSettingFile";//Project file name

    mySettings = new QSettings(WorkSettingsPath, WorkSettingsFile);
    //------------ add mySettings value below ---------
    QString filenameDefaultSettings("./" + WorkSettingsPath + "/" + WorkSettingsFile + ".conf");

    //Load settings ******
    //spinValueTableRows = mySettings->value("mySettings/spinValueTableRows", "").toInt();
    //mainPowerVoltageSet = mySettings->value("mySettings/mainPowerVoltageSet", "").toFloat();
    Mixer_inhouse_1 = mySettings->value("mySettings/Mixer_inhouse_1", "").toFloat();
    PID_forward_gain = mySettings->value("mySettings/PID_forward_gain", "").toFloat();
    PID_forward_offset = mySettings->value("mySettings/PID_forward_offset", "").toFloat();
    PID_par_cvu = mySettings->value("mySettings/PID_par_cvu", "").toFloat();
    PID_par_cvl = mySettings->value("mySettings/PID_par_cvl", "").toFloat();
    PID_par_p = mySettings->value("mySettings/PID_par_p", "").toFloat();
    PID_par_i = mySettings->value("mySettings/PID_par_i", "").toFloat();
    PID_par_d = mySettings->value("mySettings/PID_par_d", "").toFloat();
    PID_par_tau_i = mySettings->value("mySettings/PID_par_tau_i", "").toFloat();
    PID_par_tau_d = mySettings->value("mySettings/PID_par_tau_d", "").toFloat();
    temp_setp_1 = mySettings->value("mySettings/temp_setp_1", "").toFloat();
    ui->doubleSpinBox_inhouse_setp->setValue(temp_setp_1);
    ui->doubleSpinBox_gain_forward->setValue(PID_forward_gain);
    ui->doubleSpinBox_offset_forward->setValue(PID_forward_offset);


    ui->spinBox_mixer->setValue(Mixer_inhouse_1);
    for(int i=0;i<NR_TEMP_SENSOR_GUI;i++){
        temp_connection_matrix.push_back(i);
        temperature_matrix.push_back(0.0);
        temperature_inp.push_back(0.0);
    }
    for(int i=0;i<temp_connection_matrix.size();i++)
    {
        temp_connection_matrix[i] = mySettings->value(QString("mySettings/temp_connection_matrix%1").arg(i), "").toInt();
        switch(i){
        case(0):
            ui->spinBox_temp_to_inhouse->setValue(temp_connection_matrix[i]);
        break;
        case(1):
            ui->spinBox_temp_to_inhouse_2->setValue(temp_connection_matrix[i]);
        break;
        case(2):
            ui->spinBox_temp_to_inhouse_3->setValue(temp_connection_matrix[i]);
        break;
        case(3):
            ui->spinBox_temp_to_inhouse_4->setValue(temp_connection_matrix[i]);
        break;
        case(4):
            ui->spinBox_temp_to_inhouse_5->setValue(temp_connection_matrix[i]);
        break;
        case(5):
            ui->spinBox_temp_to_inhouse_6->setValue(temp_connection_matrix[i]);
        break;
        case(6):
            ui->spinBox_temp_to_inhouse_7->setValue(temp_connection_matrix[i]);
        break;
        case(7):
            ui->spinBox_temp_to_inhouse_8->setValue(temp_connection_matrix[i]);
        break;
        case(8):
            ui->spinBox_temp_to_inhouse_9->setValue(temp_connection_matrix[i]);
        break;
        case(9):
            ui->spinBox_temp_to_inhouse_10->setValue(temp_connection_matrix[i]);
        break;
        }
    }



    //start_up = 1;
    //QPixmap radiator_pix("./radiator.png");
    //QPixmap tap_water_pix("./tap_water.png");
    radiator_pix = new QPixmap;
    radiator_pix->load("./radiator.png");
    tap_water_pix = new QPixmap;
    tap_water_pix->load("./tap_water.png");
    OFF_pix = new QPixmap;
    OFF_pix->load("./OFF.png");
    Plus_pix = new QPixmap;
    Plus_pix->load("./Plus2.png");
    Plus_pix_b = new QPixmap;
    Plus_pix_b->load("./Plus2.png");

    Minus_pix = new QPixmap;
    Minus_pix->load("./Minus.png");
    error_pix = new QPixmap;
    error_pix->load("./error.png");

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
            defval=32;
        }
        heatpump_send.push_back(defval);
        heatpump_reply.push_back(0);
    }

    controller *controlobj;
    controlobj = new controller;
   // ui->lineEdit_T1->setValue(0.0f);

    connect(controlobj, SIGNAL(controllertick(void)), tempsobj, SLOT(gettemperature(void)));
    connect(controlobj, SIGNAL(controllertick(void)), this, SLOT(controllertick(void)));
    connect(tempsobj, SIGNAL(Temperature(QVector<float>)), this, SLOT(temperatures(QVector<float>)));
    connect(tempsobj, SIGNAL(Rom_vect(QVector<QString>)), this, SLOT(temp_id(QVector<QString>)));
    connect(this, SIGNAL(setheatpump(QVector<int>)), heatpobj, SLOT(setheatpump(QVector<int>)));
    connect(this, SIGNAL(test(void)), heatpobj, SLOT(test(void)));
    connect(heatpobj, SIGNAL(replyheatpump(QVector<int>)), this, SLOT(heatpumpreply(QVector<int>)));


    mySettings->sync();//save mySettings

    //ui->label_pic->setPixmap(radiator_pix->scaled(60,60));
    //radiator_pix->load("./radiator.png");
    *radiator_pix = radiator_pix->scaled(60,60);
    //tap_water_pix->load("./tap_water.png");
    *tap_water_pix = tap_water_pix->scaled(60,60);
    //OFF_pix->load("./OFF.png");
    *OFF_pix = OFF_pix->scaled(60,60);
    *Plus_pix = Plus_pix->scaled(60,50);
    *Plus_pix_b = Plus_pix_b->scaled(60,50);
    *Minus_pix = Minus_pix->scaled(60,50);
    *error_pix = error_pix->scaled(60,50);

    ui->plus_pix->setPixmap(*Plus_pix);
    ui->plus_pix_2->setPixmap(*Plus_pix);
    ui->minus_pix->setPixmap(*Minus_pix);
    ui->label_pic->setPixmap(*OFF_pix);
    ui->spinBox_manual_hotwater->setValue(55);
    ui->spinBox_man_temp_hp->setValue(30);
    auto_mode_turn_on();
    mySettings->sync();//save mySettings

}


void MainWindow::temperatures(QVector<float> tempvector)
{
    for(int i=0;i<tempvector.size();i++)
    {

        double temperat_l = (double)tempvector[i];
        if(i >= temperature_inp.size())
        {
            printf("Internal program error code 100-01\n");
            printf("tempvector.size() = %d, temperature_inp.size() = %d\n", tempvector.size(), temperature_inp.size());
        }
        else
        {
            temperature_inp[i] = temperat_l;
        }


        switch (i) {
        case 0:
            ui->lineEdit_T1->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 1:
            ui->lineEdit_T2->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 2:
            ui->lineEdit_T3->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 3:
            ui->lineEdit_T4->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 4:
            ui->lineEdit_T5->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 5:
            ui->lineEdit_T6->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 6:
            ui->lineEdit_T7->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 7:
            ui->lineEdit_T8->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 8:
            ui->lineEdit_T9->setText(QString::number(temperat_l, 'f', 3));
            break;
        case 9:
            ui->lineEdit_T10->setText(QString::number(temperat_l, 'f', 3));
            break;

        }
    }
    for(int i=0;i<temp_connection_matrix.size();i++)
    {
        int x = temp_connection_matrix[i]-1;
        if(x < 0 || x > temperature_inp.size()-1){
            printf("Internal program error code 100-02\n");
            printf("temp_connection_matrix[%d] = %d, temperature_inp.size() = %d\n", i, temp_connection_matrix[i], temperature_inp.size());
        }
        else {
            temperature_matrix[i] = temperature_inp[x];
        }
        switch (i) {
        case 0:
            outside_temp = temperature_inp[x];
            ui->lineEdit_tx1->setText(QString::number(outside_temp, 'f', 3));
            ui->lineEdit_tx1_2->setText(QString::number(outside_temp, 'f', 3));
            break;
        case 1:
            ui->lineEdit_tx2->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 2:
            ui->lineEdit_tx3->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 3:
            ui->lineEdit_tx4->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 4:
            ui->lineEdit_tx5->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 5:
            ui->lineEdit_tx6->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 6:
            ui->lineEdit_tx7->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 7:
            ui->lineEdit_tx8->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 8:
            ui->lineEdit_tx9->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 9:
            ui->lineEdit_tx10->setText(QString::number(temperature_inp[x], 'f', 3));
            break;

        }

    }
    inhouse_temp = (((double)Mixer_inhouse_1) * 0.01 * temperature_matrix[1]) + (((double)Mixer_inhouse_2) * 0.01 * temperature_matrix[2]);


    ui->lineEdit_mixed_inhouse->setText(QString::number(inhouse_temp, 'f', 3));
    ui->lineEdit_feedback->setText(QString::number(inhouse_temp, 'f', 2));
    temp_setp_with_profile = temp_setp_1 + temp_profile;
    ui->lineEdit_setpoint->setText(QString::number(temp_setp_with_profile, 'f', 2));
    ui->lineEdit_setpoint_2->setText(QString::number(temp_setp_with_profile, 'f', 2));
    forward_temp = temp_setp_with_profile - temperature_matrix[0];
    ui->lineEdit_forward->setText(QString::number(forward_temp, 'f', 2));

    forward_signal = (forward_temp * PID_forward_gain) + PID_forward_offset;
    ui->lineEdit_forward_2->setText(QString::number(forward_signal, 'f', 2));
   //printf("inhouse_temp =%f\n", (float)inhouse_temp);
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

    //Example save data to mySettings
    //QString y = QString::number((double)parameter1, 10, 2);
    //mySettings->setValue(QString("mySettings/parameter1"), y);
    //mySettings->setValue(QString("mySettings/TableLabelRow%1").arg(rows), QString("Label %1").arg(rows+1));
    //mySettings->setValue("mySettings/IPnumber", IPnumber);

    //Set date and time stamp in Project file
    //QDateTime dateTime = QDateTime::currentDateTime();
    //String dateTimeString = dateTime.toString();
    //mySettings->setValue("mySettings/x_Date_Time_Project_Modify", dateTimeString);
    for(int i=0;i<temp_connection_matrix.size();i++)
    {
        mySettings->setValue(QString("mySettings/temp_connection_matrix%1").arg(i), temp_connection_matrix[i]);
    }

    QString y = QString::number((double)Mixer_inhouse_1, 10, 2);
    mySettings->setValue(QString("mySettings/Mixer_inhouse_1"), y);
    y = QString::number(PID_forward_gain, 10, 9);
    mySettings->setValue(QString("mySettings/PID_forward_gain"), y);
    y = QString::number(PID_forward_offset, 10, 9);
    mySettings->setValue(QString("mySettings/PID_forward_offset"), y);

    y = QString::number(PID_par_cvu, 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_cvu"), y);
    y = QString::number(PID_par_cvl, 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_cvl"), y);
    y = QString::number(PID_par_p, 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_p"), y);
    y = QString::number(PID_par_i, 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_i"), y);
    y = QString::number(PID_par_d, 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_d"), y);
    y = QString::number(PID_par_tau_i, 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_tau_i"), y);
    y = QString::number(PID_par_tau_d, 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_tau_d"), y);
    y = QString::number(temp_setp_1, 10, 9);
    mySettings->setValue(QString("mySettings/temp_setp_1"), y);

    mySettings->sync();//save mySettings
    delete ui;
}

void MainWindow::on_checkBox_on_off_clicked(bool checked)
{
    ui->checkBox_on_off->setChecked(checked);
    if(checked==false){
        gray_out_user();
        ui->checkBox_heater_mode->setChecked(false);
        ui->checkBox_hotwater_mode->setChecked(false);
        ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    }
}

void MainWindow::on_spinBox_man_temp_hp_valueChanged(int arg1)
{
    //heatpump_send[0] = 6;//Set temperature
    //heatpump_send[1] = arg1;//Manual temperature set heatpump
    //ui->checkBox_heater_mode->setChecked(true);
    //ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    //ui->checkBox_hotwater_mode->setChecked(false);
    //emit setheatpump(heatpump_send);
    //printf("heatpump_send[1] = %d\n", heatpump_send[1]);
}


void MainWindow::on_checkBox_hotwater_mode_clicked(bool checked)
{
    if(checked == true){
        ui->checkBox_heater_mode->setChecked(false);
        ui->checkBox_hotwater_and_heater_mode->setChecked(false);
        ui->spinBox_man_temp_hp->setEnabled(false);
        //ui->spinBox_manual_hotwater->setEnabled(true);

        //ui->checkBox_hotwater_mode->setChecked(true);
    }
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
        //ui->checkBox_heater_mode->setChecked(true);
        ui->checkBox_hotwater_and_heater_mode->setChecked(false);
        ui->checkBox_hotwater_mode->setChecked(false);

        ui->spinBox_manual_hotwater->setEnabled(false);
        bool checkbox_state = ui->checkBox_auto->checkState();
        if(checkbox_state == true){
            ui->spinBox_man_temp_hp->setEnabled(false);
        }
    }
}

void MainWindow::on_spinBox_manual_hotwater_valueChanged(int arg1)
{
    //heatpump_send[0] = 6;//Set temperature
    //heatpump_send[1] = arg1;//Manual temperature set heatpump
    //emit setheatpump(heatpump_send);
    //printf("heatpump_send[1] = %d\n", heatpump_send[1]);
    //ui->checkBox_heater_mode->setChecked(false);
    //ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    //ui->checkBox_hotwater_mode->setChecked(true);

//    ui->spinBox_man_temp_hp->setEnabled(true);
//    ui->spinBox_manual_hotwater->setEnabled(true);

}

void MainWindow::on_checkBox_hotwater_and_heater_mode_clicked(bool checked)
{
    if(checked==true){
        ui->checkBox_heater_mode->setChecked(false);
        //ui->checkBox_hotwater_and_heater_mode->setChecked(true);
        ui->checkBox_hotwater_mode->setChecked(false);
        ui->spinBox_man_temp_hp->setEnabled(false);
        ui->spinBox_manual_hotwater->setEnabled(false);
    }
}

void MainWindow::on_checkBox_manual_clicked(bool checked)
{
    if(checked==true){

        ui->checkBox_auto->setChecked(false);
        ui->checkBox_auto->setEnabled(true);
        ui->checkBox_manual->setEnabled(false);
        ui->checkBox_on_off->setEnabled(true);

        bool checkbox_state = ui->checkBox_on_off->checkState();
        if(checkbox_state == true)
        {
            ui->checkBox_hotwater_mode->setEnabled(true);
            ui->checkBox_hotwater_and_heater_mode->setEnabled(true);
            ui->checkBox_heater_mode->setEnabled(true);
            ui->spinBox_man_temp_hp->setEnabled(false);
            ui->spinBox_manual_hotwater->setEnabled(false);
        }
        switch (heatpump_reply[REPLY_INDEX_7_u4]) {
        case(REPLY_I7_hot_hotwater_33):
            ui->checkBox_heater_mode->setChecked(false);
            ui->checkBox_hotwater_and_heater_mode->setChecked(true);
            ui->checkBox_hotwater_mode->setChecked(false);
            break;
        case(REPLY_I7_hotwater_44):
            ui->checkBox_heater_mode->setChecked(false);
            ui->checkBox_hotwater_and_heater_mode->setChecked(false);
            ui->checkBox_hotwater_mode->setChecked(true);
            break;
        case(REPLY_I7_hot_55):
            ui->checkBox_heater_mode->setChecked(true);
            ui->checkBox_hotwater_and_heater_mode->setChecked(false);
            ui->checkBox_hotwater_mode->setChecked(false);
            break;
        }
    }
}

void MainWindow::auto_mode_turn_on()
{
    ui->checkBox_auto->setEnabled(false);
    ui->checkBox_manual->setEnabled(true);
    ui->checkBox_on_off->setEnabled(false);
    ui->checkBox_manual->setChecked(false);
    gray_out_user();
    start_up = 1;
    auto_init_done = 0;
}
void MainWindow::on_checkBox_auto_clicked(bool checked)
{
    if(checked==true){
        auto_mode_turn_on();
    }
}
void MainWindow::gray_out_user()
{
    ui->checkBox_hotwater_mode->setEnabled(false);
    ui->checkBox_heater_mode->setEnabled(false);
    ui->checkBox_hotwater_and_heater_mode->setEnabled(false);
    ui->spinBox_man_temp_hp->setEnabled(false);
    ui->spinBox_manual_hotwater->setEnabled(false);
}

void MainWindow::heatpumpreply(QVector<int> arg1)
{
    if(heatpump_reply != arg1)
    {
        heatpump_reply = arg1;
        ui->lcdnumber_both_cmd_2->display(heatpump_reply[REPLY_INDEX_4_SETP_TEMP]);
        ui->lcdnumber_both_actual_2->display(heatpump_reply[REPLY_INDEX_3_ACTUAL_TEMP]);

        ui->label_32->setNum(heatpump_reply[REPLY_INDEX_6_u5]);
        ui->label_34->setNum(heatpump_reply[REPLY_INDEX_7_u4]);
        ui->label_36->setNum(heatpump_reply[REPLY_INDEX_8_u1]);
        ui->label_77->setNum(heatpump_reply[REPLY_INDEX_9_uError]);
    }

}
void MainWindow::controllertick(void)
{
    bool checkbox_auto = ui->checkBox_auto->checkState();
    bool checkbox_radiator_mode = ui->checkBox_heater_mode->checkState();
    bool checkbox_tap_water_mode = ui->checkBox_hotwater_mode->checkState();
    bool checkbox_both_mode = ui->checkBox_hotwater_and_heater_mode->checkState();

    emit setheatpump(heatpump_send);
    if(heatpump_reply[REPLY_INDEX_9_uError] == REPLY_I9_NETWORK_OK){
        //Network OK

        //REPLY_I7_hot_55
        if(heatpump_reply[REPLY_INDEX_8_u1] == REPLY_I8_22_ON){
            //Heatpump is ON

            switch(heatpump_reply[REPLY_INDEX_6_u5])
            {
            case(REPLY_I6_Hot_555):
                ui->label_pic->setPixmap(*radiator_pix);

                break;

            case(REPLY_I6_HotWater_444):
                ui->label_pic->setPixmap(*tap_water_pix);
                break;
            }

 //*******
            if(checkbox_auto == false){
                //Manual mode
                //heatpump_send[CMD_INDEX_0] = CMD_hot_hotwater;
                if(checkbox_radiator_mode == true)
                {
                    if(heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hot_55){
                       ui->spinBox_man_temp_hp->setEnabled(true);
                       heatpump_send[CMD_INDEX_0] = CMD_SET_TEMP;//set_temp
                       heatpump_send[SET_INDEX_1_SETP_TEMP] = ui->spinBox_man_temp_hp->value();
                    }
                    else
                    {
                       ui->spinBox_man_temp_hp->setEnabled(false);
                       heatpump_send[CMD_INDEX_0] = CMD_hot;
                    }
                }
                else if (checkbox_tap_water_mode == true) {
                    if(heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hotwater_44){
                       ui->spinBox_manual_hotwater->setEnabled(true);
                       heatpump_send[CMD_INDEX_0] = CMD_SET_TEMP;//set_temp
                       heatpump_send[SET_INDEX_1_SETP_TEMP] = ui->spinBox_manual_hotwater->value();
                    }
                    else
                    {
                       ui->spinBox_manual_hotwater->setEnabled(false);
                       heatpump_send[CMD_INDEX_0] = CMD_hotwater;
                    }
                }
                else if (checkbox_both_mode == true){
                    ui->spinBox_man_temp_hp->setEnabled(false);
                    ui->spinBox_manual_hotwater->setEnabled(false);
                    heatpump_send[CMD_INDEX_0] = CMD_hot_hotwater;//both mode
                }
            }
            else{
                //Auto mode
                if(auto_init_done == true)
                {
                    if(heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hot_55){
                        ui->checkBox_heater_mode->setChecked(true);
                        ui->checkBox_hotwater_mode->setChecked(false);
                        ui->checkBox_hotwater_and_heater_mode->setChecked(false);
                    }
                    else if (heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hotwater_44) {
                        ui->checkBox_heater_mode->setChecked(false);
                        ui->checkBox_hotwater_mode->setChecked(true);
                        ui->checkBox_hotwater_and_heater_mode->setChecked(false);
                    }
                    else if (heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hot_hotwater_33) {
                        ui->checkBox_heater_mode->setChecked(false);
                        ui->checkBox_hotwater_mode->setChecked(false);
                        ui->checkBox_hotwater_and_heater_mode->setChecked(true);
                    }
                    else{
                        ui->checkBox_heater_mode->setChecked(false);
                        ui->checkBox_hotwater_mode->setChecked(false);
                        ui->checkBox_hotwater_and_heater_mode->setChecked(false);
                    }

                    if(heatpump_reply[REPLY_INDEX_6_u5] == REPLY_I6_Hot_555){//Heatpump is now in radiator mode
                        if(heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hot_55)
                        {//Heatpump is now in Commanded radiator mode
                            if(heatpump_reply[REPLY_INDEX_4_SETP_TEMP] == ui->spinBox_man_temp_hp->value()){
                                heatpump_send[CMD_INDEX_0] = CMD_hot_hotwater;//Switch back to both mode when commanded setp temp is equal to spinBox value setp
                            }
                            else{
                                heatpump_send[CMD_INDEX_0] = CMD_SET_TEMP;//set_temp
                                heatpump_send[SET_INDEX_1_SETP_TEMP] = ui->spinBox_man_temp_hp->value();
                            }
                        }
                        else
                        {//Heatpump is NOT in Commanded radiator mode, now heatpump is in tap water mode or both mode
                            if(heatpump_reply[REPLY_INDEX_4_SETP_TEMP] != ui->spinBox_man_temp_hp->value()){
                                heatpump_send[CMD_INDEX_0] = CMD_hot;
                            }
                        }
                    }
                    else{//Heatpump is NOT in radiator mode,  NOT REPLY_I6_Hot_555 mode
                        //Wait until heater have jump over to radiator mode when tap water is heated up
                        //We could send both mode also when waiting
                        heatpump_send[CMD_INDEX_0] = CMD_hot_hotwater;//both mode
                    }
                }
            }
 //*******
        }
        else if(heatpump_reply[REPLY_INDEX_8_u1] == REPLY_I8_11_OFF)
        {
            //OFF
            ui->label_pic->setPixmap(*OFF_pix);
        }
        else{
            ui->label_pic->setPixmap(*error_pix);
        }
        //Start up cycle
        switch(start_up){
        case(0):
            //Do nothing with start up when 0

            break;
        case(1):
            gray_out_user();
            if(heatpump_reply[REPLY_INDEX_8_u1] == REPLY_I8_11_OFF)
            {
                //device is OFF
                heatpump_send[CMD_INDEX_0] = CMD_TURN_ON;//Turn ON
                ui->checkBox_on_off->setChecked(true);
            }
            if(heatpump_reply[REPLY_INDEX_8_u1] == REPLY_I8_22_ON)
            {
                //device is ON
                tick_cnt1++;
                if(tick_cnt1>3){
                    start_up++;
                }
            }


            break;
        case(2):
            set_radiator_mode();
            heatpump_send[CMD_INDEX_0] = CMD_hot;
            tick_cnt1++;
            if(tick_cnt1>3){
                start_up++;
            }

            break;
        case(3):

            if(heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hot_55)
            {
                //device is radiator mode reply
                heatpump_send[CMD_INDEX_0] = CMD_SET_TEMP;//Set temperature
                heatpump_send[SET_INDEX_1_SETP_TEMP] = ui->spinBox_man_temp_hp->value();
                start_up++;
                tick_cnt1 = 0;
            }
            break;
        case(4):

            if(heatpump_reply[REPLY_INDEX_4_SETP_TEMP] == ui->spinBox_man_temp_hp->value())
            {
                //radiator setpoint reply is equal to Spinbox Send radiator setpoint
                tick_cnt1++;
                if(tick_cnt1>4){
                    start_up++;
                }
            }
            break;

        case(5):
            set_tap_water_mode();
            heatpump_send[CMD_INDEX_0] = CMD_hotwater;
            tick_cnt1++;
            if(tick_cnt1>4){
                start_up++;
            }

            break;
        case(6):

            if(heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hotwater_44)
            {
                //device is tap water mode reply
                heatpump_send[CMD_INDEX_0] = CMD_SET_TEMP;//Set temperature
                heatpump_send[SET_INDEX_1_SETP_TEMP] = ui->spinBox_manual_hotwater->value();
                tick_cnt1++;
                if(tick_cnt1>4){
                    start_up++;
                }
            }
            break;
        case(7):

            if(heatpump_reply[REPLY_INDEX_4_SETP_TEMP] == ui->spinBox_manual_hotwater->value())
            {
                //tap water setpoint reply is equal to Spinbox Send tap water setpoint
                tick_cnt1++;
                if(tick_cnt1>4){
                    start_up++;
                }
            }
            break;
        case(8):
            set_both_mode();
            heatpump_send[CMD_INDEX_0] = CMD_hot_hotwater;
            start_up++;
            tick_cnt1 = 0;

            break;
        case(9):

            if(heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hot_hotwater_33)
            {
                auto_init_done = true;
                start_up = 0;
                tick_cnt1 = 0;
            }
            break;

        default:
            auto_mode_turn_on();
            tick_cnt1 = 0;
            break;
        }

        //End start up
    }
    else {
        if(heatpump_reply[REPLY_INDEX_9_uError] == REPLY_I9_NETWORK_ERR){
            //NETWORK ERROR
            ui->label_pic->setPixmap(*error_pix);
            auto_mode_turn_on();
        }
    }
}
void MainWindow::set_radiator_mode(void)
{
    //heatpump_send[0] = CMD_hot;//5= Radiator mode
    ui->checkBox_heater_mode->setChecked(true);
    ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    ui->checkBox_hotwater_mode->setChecked(false);
}
void MainWindow::set_radiator_temp(int arg1)
{

}
void MainWindow::set_tap_water_mode(void)
{
    //heatpump_send[0] = CMD_hotwater;//4= Tap water mode
    ui->checkBox_heater_mode->setChecked(false);
    ui->checkBox_hotwater_and_heater_mode->setChecked(false);
    ui->checkBox_hotwater_mode->setChecked(true);
}
void MainWindow::set_tap_water_temp(int arg1)
{

}
void MainWindow::set_both_mode(void)
{
    //heatpump_send[0] = CMD_hot_hotwater;//Radiator + tap water mode.
    ui->checkBox_heater_mode->setChecked(false);
    ui->checkBox_hotwater_and_heater_mode->setChecked(true);
    ui->checkBox_hotwater_mode->setChecked(false);
}

void MainWindow::on_checkBox_heater_mode_toggled(bool checked)
{
    if(checked==true){
        //heatpump_send[0] = CMD_hot;//5=hot mode (Only heater mode)
        //heatpump_send[0] = 6;//Set temperature
        //heatpump_send[1] = ui->spinBox_man_temp_hp->value();
    }
   // emit setheatpump(heatpump_send);
}

void MainWindow::on_checkBox_hotwater_mode_toggled(bool checked)
{
    if(checked==true){
        //heatpump_send[0] = CMD_hotwater;//4=hotwater mode (Only hotwater mode selected)
        //heatpump_send[0] = 6;//Set temperature
        //heatpump_send[1] = ui->spinBox_manual_hotwater->value();
     }
    //emit setheatpump(heatpump_send);
}

void MainWindow::on_checkBox_hotwater_and_heater_mode_toggled(bool checked)
{
    if(checked==true){
      //  heatpump_send[0] = CMD_hot_hotwater;//3=hot_hotwater mode (heater + hotwater mode selected)
    }
    //emit setheatpump(heatpump_send);

}

void MainWindow::on_checkBox_on_off_toggled(bool checked)
{
    if(checked == true){
        heatpump_send[CMD_INDEX_0] = CMD_TURN_ON;//2=Turn ON heatpump device
    }
    else{
        heatpump_send[CMD_INDEX_0] = CMD_TURN_OFF;//
    }
    emit setheatpump(heatpump_send);
    printf("heatpump_send[0] = %d\n", heatpump_send[0]);

}


void MainWindow::on_spinBox_temp_to_inhouse_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>0 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[0] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }
}

void MainWindow::on_spinBox_temp_to_inhouse_2_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>1 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[1] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_spinBox_temp_to_inhouse_3_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>2 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[2] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_spinBox_temp_to_inhouse_4_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>3 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[3] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_spinBox_temp_to_inhouse_5_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>4 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[4] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_spinBox_temp_to_inhouse_6_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>5 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[5] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_spinBox_temp_to_inhouse_7_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>6 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[6] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_spinBox_temp_to_inhouse_8_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>7 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[7] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_spinBox_temp_to_inhouse_9_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>8 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[8] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_spinBox_temp_to_inhouse_10_valueChanged(int arg1)
{
    if(temp_connection_matrix.size()>9 && arg1 > 0 && arg1 <= temp_connection_matrix.size()){
       temp_connection_matrix[9] = arg1;
    }
    else {
        printf("internal error, size of temp_connection_matrix[i] vector to small or temp_connection_matrix.size() is outside range of GUI max min\n");
        exit(0);
    }

}

void MainWindow::on_doubleSpinBox_pid_p_valueChanged(double arg1)
{

}

void MainWindow::on_spinBox_mixer_valueChanged(int arg1)
{
    if(arg1 > 100){
        arg1=100;
    }
    if(arg1 < 0){
        arg1=0;
    }
    Mixer_inhouse_1 = arg1;
    Mixer_inhouse_2 = 100 - Mixer_inhouse_1;
    ui->lineEdit_mix_2->setText(QString::number((double)Mixer_inhouse_2, 'f', 0));

}

void MainWindow::on_doubleSpinBox_gain_forward_valueChanged(double arg1)
{
    PID_forward_gain = arg1;
}

void MainWindow::on_doubleSpinBox_offset_forward_valueChanged(double arg1)
{
    PID_forward_offset = arg1;
}

void MainWindow::on_doubleSpinBox_inhouse_setp_valueChanged(double arg1)
{
    temp_setp_1 = arg1;
}
