#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<pthread.h>
#include"tempsignals.h"
#include"heatpump.h"
#include"controller.h"
#include<QVector>
#include<QPixmap>
#include <QFileDialog>
#include <wiringPi.h>
#include "shunt2_controller.h"
//#include "shunt3_controller.h"
#include "gpio_pin_map.h"
#include "view_shunt3_regulator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#define USE_85C_GLITCH_FIX
#ifdef USE_85C_GLITCH_FIX
int Error_cnt_85C =0;
#endif
#define NR_TEMP_SENSOR_GUI 10
#define HYSTERESIS_LEVEL 0.7

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

#define REINIT_TIME 3600

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)



{
     ui->setupUi(this);
     solar2pool_state = false;
    pump3_hyst = 0.0;
    alive_GPIO = 0;//Toggle every sec
    fire_auto_off_timer = 0;
    //-- GPIO settings --
    wiringPiSetup() ;
    pinMode (LED_PROGRAM_ALIVE_WATCH_DOG, OUTPUT) ;
    pinMode (LED_SHUNT2_ADJUST_BY_REGULATOR_2, OUTPUT) ;
    pinMode (RELAY_PUMP2, OUTPUT) ;
    pinMode (RELAY_INV_PUMP3, OUTPUT) ;
    pinMode (RELAY_SHUNT2_CW, OUTPUT) ;
    pinMode (RELAY_SHUNT2_CCW, OUTPUT) ;
    pinMode (RELAY_INV_PUMP2, OUTPUT) ;
    pinMode (INPUT_SHUNT2_HALF_WAY_HALL_SWITCH, INPUT) ;
    digitalWrite (LED_PROGRAM_ALIVE_WATCH_DOG,  LOW) ;
    //-------------------
    ui->checkBox_pump2->setEnabled(false);
    ui->checkBox_pump3->setEnabled(false);
    ui->checkBox_shunt2_cw->setEnabled(false);
    ui->checkBox_shunt2_ccw->setEnabled(false);
    digitalWrite (RELAY_PUMP2,  HIGH) ;
    digitalWrite (RELAY_INV_PUMP2,  LOW) ;
    ui->checkBox_pump2->setChecked(true);
    digitalWrite (RELAY_INV_PUMP3,  LOW) ;
    ui->checkBox_pump3->setChecked(true);
    digitalWrite (RELAY_SHUNT2_CW,  LOW) ;
    ui->checkBox_shunt2_cw->setChecked(false);
    digitalWrite (RELAY_SHUNT2_CCW,  LOW) ;
    ui->checkBox_shunt2_ccw->setChecked(false);


    debug_reinit_low_temp_hot_w = 0;
    reinit_timer = 0;
    hysteres_auto_off = HYSTERESIS_LEVEL;


    QFont font("Courier New");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    QApplication::setFont(font);

    ui->lcdNumber_clock->setSegmentStyle(QLCDNumber::Filled);
    ui->lcdNumber_day_profile_temp->setSegmentStyle(QLCDNumber::Filled);

//Store GUI settings
    switch_mode = false;
    temp_profile = 0.0;
    outside_temp = 0.0;
    auto_init_done = false;
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, "./");
    WorkSettingsPath = "WorkSettingFolder";//WorkSettingsPath path
    WorkSettingsFile = "WorkSettingFile";//Project file name

    mySettings = new QSettings(WorkSettingsPath, WorkSettingsFile);
    //------------ add mySettings value below ---------
    QString filenameDefaultSettings("./" + WorkSettingsPath + "/" + WorkSettingsFile + ".conf");
    //Load settings ******
    //int_example = mySettings->value("mySettings/int_example", "").toInt();
    //float_example = mySettings->value("mySettings/float_example", "").toFloat();
    //double_example = mySettings->value("mySettings/double_example", "").toDouble();
    Mixer_inhouse_1 = mySettings->value("mySettings/Mixer_inhouse_1", "").toInt();
    ui->spinBox_pid_control_samp->setValue(mySettings->value("mySettings/PID_update_strobe", "").toInt());
    ui->doubleSpinBox_inhouse_setp->setValue(mySettings->value("mySettings/temp_setp_1", "").toDouble());
    ui->doubleSpinBox_gain_forward->setValue(mySettings->value("mySettings/PID_forward_gain", "").toDouble());
    ui->doubleSpinBox_offset_forward->setValue(mySettings->value("mySettings/PID_forward_offset", "").toDouble());
    ui->doubleSpinBox_pid_p->setValue(mySettings->value("mySettings/PID_par_p", "").toDouble());
    ui->doubleSpinBox_pid_i->setValue(mySettings->value("mySettings/PID_par_i", "").toDouble());
    ui->doubleSpinBox_pid_d->setValue(mySettings->value("mySettings/PID_par_d", "").toDouble());
    ui->spinBox_pid_cvu->setValue(mySettings->value("mySettings/PID_par_cvu", "").toInt());
    ui->spinBox_pid_cvl->setValue(mySettings->value("mySettings/PID_par_cvl", "").toInt());
    ui->spinBox_pid_reset_tau->setValue(mySettings->value("mySettings/PID_par_tau_i", "").toInt());
    ui->spinBox_pid_d_tau->setValue(mySettings->value("mySettings/PID_par_tau_d", "").toInt());
    ui->doubleSpinBox_auto_off_outside->setValue(mySettings->value("mySettings/auto_off_outside", "").toDouble());
    ui->doubleSpinBox_pump3_thres->setValue(mySettings->value("mySettings/pump3_thres", "").toDouble());
    ui->doubleSpinBox_shunt2_gain->setValue(mySettings->value("mySettings/shunt2_gain", "").toDouble());
    ui->doubleSpinBox_shunt2_hyst->setValue(mySettings->value("mySettings/shunt2_hyst", "").toDouble());
    ui->doubleSpinBox_pid_shunt2_cvu->setValue(mySettings->value("mySettings/shunt2_cvu_par", "").toDouble());
    ui->doubleSpinBox_pid_shunt2_cvl->setValue(mySettings->value("mySettings/shunt2_cvl_par", "").toDouble());
    ui->doubleSpinBox_pid_shunt2_i->setValue(mySettings->value("mySettings/shunt2_i_par", "").toDouble());
    ui->doubleSpinBox_pid_shunt2_d->setValue(mySettings->value("mySettings/shunt2_d_par", "").toDouble());
    ui->doubleSpinBox_pid_shunt2_tau_i->setValue(mySettings->value("mySettings/shunt2_tau_i", "").toDouble());
    ui->doubleSpinBox_pid_shunt2_tau_d->setValue(mySettings->value("mySettings/shunt2_tau_d", "").toDouble());

    ui->spinBox_auto_off_actual->setValue(mySettings->value("mySettings/auto_off_actual", "").toInt());
    ui->spinBox_shunt2_auto_off->setValue(mySettings->value("mySettings/shunt2_auto_off", "").toInt());
    ui->spinBox_hot_w_low_th->setValue(mySettings->value("mySettings/hot_w_low_threshold_b", "").toInt());
    ui->spinBox_high_temp_hot_w_th->setValue(mySettings->value("mySettings/high_temp_hot_w_th_b", "").toInt());
    ui->doubleSpinBox_solar_diff_on->setValue(mySettings->value("mySettings/solar_diff_on", "").toDouble());
    ui->doubleSpinBox_solar_diff_off->setValue(mySettings->value("mySettings/solar_diff_off", "").toDouble());
    ui->doubleSpinBox_max_water_top->setValue(mySettings->value("mySettings/max_water_top", "").toDouble());
    ui->doubleSpinBox_min_water_top->setValue(mySettings->value("mySettings/min_water_top", "").toDouble());

    ui->verticalSlider_0->setValue(mySettings->value("mySettings/day_houer_profile_0", "").toInt());
    ui->verticalSlider_1->setValue(mySettings->value("mySettings/day_houer_profile_1", "").toInt());
    ui->verticalSlider_2->setValue(mySettings->value("mySettings/day_houer_profile_2", "").toInt());
    ui->verticalSlider_3->setValue(mySettings->value("mySettings/day_houer_profile_3", "").toInt());
    ui->verticalSlider_4->setValue(mySettings->value("mySettings/day_houer_profile_4", "").toInt());
    ui->verticalSlider_5->setValue(mySettings->value("mySettings/day_houer_profile_5", "").toInt());
    ui->verticalSlider_6->setValue(mySettings->value("mySettings/day_houer_profile_6", "").toInt());
    ui->verticalSlider_7->setValue(mySettings->value("mySettings/day_houer_profile_7", "").toInt());
    ui->verticalSlider_8->setValue(mySettings->value("mySettings/day_houer_profile_8", "").toInt());
    ui->verticalSlider_9->setValue(mySettings->value("mySettings/day_houer_profile_9", "").toInt());
    ui->verticalSlider_10->setValue(mySettings->value("mySettings/day_houer_profile_10", "").toInt());
    ui->verticalSlider_11->setValue(mySettings->value("mySettings/day_houer_profile_11", "").toInt());
    ui->verticalSlider_12->setValue(mySettings->value("mySettings/day_houer_profile_12", "").toInt());
    ui->verticalSlider_13->setValue(mySettings->value("mySettings/day_houer_profile_13", "").toInt());
    ui->verticalSlider_14->setValue(mySettings->value("mySettings/day_houer_profile_14", "").toInt());
    ui->verticalSlider_15->setValue(mySettings->value("mySettings/day_houer_profile_15", "").toInt());
    ui->verticalSlider_16->setValue(mySettings->value("mySettings/day_houer_profile_16", "").toInt());
    ui->verticalSlider_17->setValue(mySettings->value("mySettings/day_houer_profile_17", "").toInt());
    ui->verticalSlider_18->setValue(mySettings->value("mySettings/day_houer_profile_18", "").toInt());
    ui->verticalSlider_19->setValue(mySettings->value("mySettings/day_houer_profile_19", "").toInt());
    ui->verticalSlider_20->setValue(mySettings->value("mySettings/day_houer_profile_20", "").toInt());
    ui->verticalSlider_21->setValue(mySettings->value("mySettings/day_houer_profile_21", "").toInt());
    ui->verticalSlider_22->setValue(mySettings->value("mySettings/day_houer_profile_22", "").toInt());
    ui->verticalSlider_23->setValue(mySettings->value("mySettings/day_houer_profile_23", "").toInt());
    update_profile_lable();

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

    controller* controlobj;
    controlobj = new controller;

    shunt2_controller* shunt2obj1;
    shunt2obj1 = new shunt2_controller;

    //shunt3_controller* shunt3obj1;
    shunt3obj1 = new shunt3_controller;
    shunt3obj1->mySettings = mySettings;
    shunt3obj1->init_with_mySett();

    connect(controlobj, SIGNAL(controllertick(void)), tempsobj, SLOT(gettemperature(void)));
    connect(controlobj, SIGNAL(controllertick(void)), this, SLOT(controllertick(void)));
    connect(controlobj, SIGNAL(PID_control_signal(double)), this, SLOT(PID_control_signal(double)));
    connect(controlobj, SIGNAL(PID_control_instant_signal(double)), this, SLOT(PID_control_instant_signal(double)));
    connect(controlobj, SIGNAL(PID_control_instant_signal(double)), shunt2obj1, SLOT(PID_control_instant_signal(double)));
    connect(this, SIGNAL(radiator_temp2(double)), shunt2obj1, SLOT(radiator_temp2(double)));
    connect(this, SIGNAL(shunt2_contr_ON(int)), shunt2obj1, SLOT(shunt2_contr_ON(int)));
    connect(this, SIGNAL(shunt2_gain_par(double)), shunt2obj1, SLOT(shunt2_gain_par(double)));
    connect(this, SIGNAL(shunt2_cvu_par(double)), shunt2obj1, SLOT(shunt2_cvu_par(double)));
    connect(this, SIGNAL(shunt2_cvl_par(double)), shunt2obj1, SLOT(shunt2_cvl_par(double)));
    connect(this, SIGNAL(shunt2_i_par(double)), shunt2obj1, SLOT(shunt2_i_par(double)));
    connect(this, SIGNAL(shunt2_d_par(double)), shunt2obj1, SLOT(shunt2_d_par(double)));
    connect(this, SIGNAL(shunt2_tau_i(double)), shunt2obj1, SLOT(shunt2_tau_i(double)));
    connect(this, SIGNAL(shunt2_tau_d(double)), shunt2obj1, SLOT(shunt2_tau_d(double)));

    connect(this, SIGNAL(shunt2_hyst_par(double)), shunt2obj1, SLOT(shunt2_hyst_par(double)));
    connect(shunt2obj1, SIGNAL(indicator_shunt2_cw(bool)), this, SLOT(indicator_shunt2_cw(bool)));
    connect(shunt2obj1, SIGNAL(indicator_shunt2_ccw(bool)), this, SLOT(indicator_shunt2_ccw(bool)));
    connect(shunt2obj1, SIGNAL(indicator_shunt2_d_part(double)), this, SLOT(indicator_shunt2_d_part(double)));
    connect(shunt2obj1, SIGNAL(indicator_shunt2_d_filt(double)), this, SLOT(indicator_shunt2_d_filt(double)));


    connect(this, SIGNAL(PID_p_cvu(int)), controlobj, SLOT(PID_p_cvu(int)));
    connect(this, SIGNAL(PID_p_cvl(int)), controlobj, SLOT(PID_p_cvl(int)));
    connect(this, SIGNAL(PID_p_p(double)), controlobj, SLOT(PID_p_p(double)));
    connect(this, SIGNAL(PID_p_i(double)), controlobj, SLOT(PID_p_i(double)));
    connect(this, SIGNAL(PID_p_d(double)), controlobj, SLOT(PID_p_d(double)));
    connect(this, SIGNAL(PID_p_tau_i(int)), controlobj, SLOT(PID_p_tau_i(int)));
    connect(this, SIGNAL(PID_p_tau_d(int)), controlobj, SLOT(PID_p_tau_d(int)));
    connect(this, SIGNAL(PID_feedback(double)), controlobj, SLOT(PID_feedback(double)));
    connect(this, SIGNAL(PID_setpoint(double)), controlobj, SLOT(PID_setpoint(double)));
    connect(this, SIGNAL(PID_forward(double)), controlobj, SLOT(PID_forward(double)));
    connect(this, SIGNAL(PID_update_samp(int)), controlobj, SLOT(PID_update_samp(int)));
    connect(this, SIGNAL(controller_mode(int)), controlobj, SLOT(controller_mode(int)));



    connect(tempsobj, SIGNAL(Temperature(QVector<float>)), this, SLOT(temperatures(QVector<float>)));
    connect(tempsobj, SIGNAL(Rom_vect(QVector<QString>)), this, SLOT(temp_id(QVector<QString>)));
    connect(this, SIGNAL(setheatpump(QVector<int>)), heatpobj, SLOT(setheatpump(QVector<int>)));
    connect(this, SIGNAL(test(void)), heatpobj, SLOT(test(void)));
    connect(heatpobj, SIGNAL(replyheatpump(QVector<int>)), this, SLOT(heatpumpreply(QVector<int>)));

    connect(this, SIGNAL(shunt3_contr_ON(int)), shunt3obj1, SLOT(shunt3_contr_ON(int)));
    connect(this, SIGNAL(shunt3_temperature_fb(double)), shunt3obj1, SLOT(temperature_fb(double)));


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
    //ui->spinBox_manual_hotwater->setValue(55);
    //ui->spinBox_man_temp_hp->setValue(30);
    auto_mode_turn_on();
    emit_PID_parameters();
    emit controller_mode(CONTROLLER_MODE_RESET_PID);
    emit shunt2_gain_par(ui->doubleSpinBox_shunt2_gain->value());
    emit shunt2_hyst_par(ui->doubleSpinBox_shunt2_hyst->value());
    emit shunt2_cvu_par(ui->doubleSpinBox_pid_shunt2_cvu->value());
    emit shunt2_cvl_par(ui->doubleSpinBox_pid_shunt2_cvl->value());
    emit shunt2_i_par(ui->doubleSpinBox_pid_shunt2_i->value());
    emit shunt2_d_par(ui->doubleSpinBox_pid_shunt2_d->value());
    emit shunt2_tau_i(ui->doubleSpinBox_pid_shunt2_tau_i->value());
    emit shunt2_tau_d(ui->doubleSpinBox_pid_shunt2_tau_d->value());

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
#ifdef USE_85C_GLITCH_FIX
            if(temperat_l == 85.0){
                //Error
                printf("Reading temperature sensor 85.0C maybe error\n");
                Error_cnt_85C++;
                printf("Error_cnt_85C =%d\n", Error_cnt_85C);
            }else
            {
                temperature_inp[i] = temperat_l;
            }
#elif
            temperature_inp[i] = temperat_l;
#endif

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
        double hot_w_temp_setp = 0.0;
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
            pool_exchanger = temperature_inp[x];
            break;
        case 5:
            ui->lineEdit_tx6->setText(QString::number(temperature_inp[x], 'f', 3));
            solar_exchanger = temperature_inp[x];
            break;
        case 6:
            ui->lineEdit_tx7->setText(QString::number(temperature_inp[x], 'f', 3));

            hot_w_temp_sensor = temperature_inp[x];
            hot_w_temp_setp = (double)ui->spinBox_manual_hotwater->value();
            hot_w_low_threshold_b = (double)ui->spinBox_hot_w_low_th->value();
            if((hot_w_temp_sensor + hot_w_low_threshold_b) < hot_w_temp_setp && reinit_timer == 0)
            {
                reinit_timer = REINIT_TIME;
                start_up = 1;
                auto_init_done = false;
                debug_reinit_low_temp_hot_w++;
            }

            break;
        case 7:
            ui->lineEdit_tx8->setText(QString::number(temperature_inp[x], 'f', 3));
            emit shunt3_temperature_fb(temperature_inp[x]);
            break;
        case 8:
            ui->lineEdit_tx9->setText(QString::number(temperature_inp[x], 'f', 3));
            break;
        case 9:
            ui->lineEdit_tx10->setText(QString::number(temperature_inp[x], 'f', 3));
            emit radiator_temp2(temperature_inp[x]);//send radiator temperature 2 sensor to shunt2 regulator
            break;

        }

    }
    inhouse_temp = (((double)Mixer_inhouse_1) * 0.01 * temperature_matrix[1]) + (((double)Mixer_inhouse_2) * 0.01 * temperature_matrix[2]);


    ui->lineEdit_mixed_inhouse->setText(QString::number(inhouse_temp, 'f', 3));
    ui->lineEdit_feedback->setText(QString::number(inhouse_temp, 'f', 2));
    temp_setp_with_profile = ui->doubleSpinBox_inhouse_setp->value() + temp_profile;
    ui->lineEdit_setpoint->setText(QString::number(temp_setp_with_profile, 'f', 2));
    ui->lineEdit_setpoint_2->setText(QString::number(temp_setp_with_profile, 'f', 2));
    forward_temp = temp_setp_with_profile - temperature_matrix[0];
    ui->lineEdit_forward->setText(QString::number(forward_temp, 'f', 2));

    forward_signal = (forward_temp * ui->doubleSpinBox_gain_forward->value()) + ui->doubleSpinBox_offset_forward->value();
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
    save_dialog_quest();
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
    else {

    }
}

void MainWindow::on_spinBox_man_temp_hp_valueChanged(int arg1)
{

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
        auto_init_done = false;
        start_up = 0;

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
void MainWindow::man_mode_checkbox_update(void)
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
}

void MainWindow::solar_to_pool(void){
    bool check_rb_cw = ui->checkBox_shunt2_cw->checkState();
    if(check_rb_cw == false){
        digitalWrite (RELAY_SHUNT2_CCW,  HIGH) ;
        ui->checkBox_shunt2_ccw->setChecked(true);
    }
    digitalWrite (RELAY_SHUNT2_CW,  LOW) ;
    ui->checkBox_shunt2_cw->setChecked(false);
    digitalWrite (RELAY_PUMP2,  HIGH) ;
    digitalWrite (RELAY_INV_PUMP2,  LOW) ;
    ui->checkBox_pump2->setChecked(true);
}

void MainWindow::set_shunt2to_cw(void){
    bool check_rb_ccw = ui->checkBox_shunt2_ccw->isChecked();
    if(check_rb_ccw == false){
        digitalWrite (RELAY_SHUNT2_CW,  HIGH) ;
        ui->checkBox_shunt2_cw->setChecked(true);
    }
    digitalWrite (RELAY_SHUNT2_CCW,  LOW) ;
    ui->checkBox_shunt2_ccw->setChecked(false);
}
void MainWindow::solar_to_hotwater(void){
    set_shunt2to_cw();
    if((solar_exchanger - solar_top_diff_ON ) > hot_w_temp_sensor){
        digitalWrite (RELAY_PUMP2,  HIGH) ;
        digitalWrite (RELAY_INV_PUMP2,  LOW) ;
        ui->checkBox_pump2->setChecked(true);
    }
    if(solar_exchanger < (hot_w_temp_sensor + solar_top_diff_OFF )){
        digitalWrite (RELAY_PUMP2,  LOW) ;
        digitalWrite (RELAY_INV_PUMP2,  HIGH) ;
        ui->checkBox_pump2->setChecked(false);
    }
//    printf("Debug solar_exchanger = %f \n", solar_exchanger);
//    printf("Debug solar_top_diff_ON = %f \n", solar_top_diff_ON);
//    printf("Debug hot_w_temp_sensor = %f \n", hot_w_temp_sensor);
//    printf("Debug solar_top_diff_OFF = %f \n", solar_top_diff_OFF);
}

void MainWindow::controllertick(void)
{

    //Clock
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm");
    if ((time.second() % 2) == 0)
        text[2] = ' ';
    ui->lcdNumber_clock->display(text);
    //end clock
    if(alive_GPIO > 0){
        alive_GPIO=0;
        digitalWrite (LED_PROGRAM_ALIVE_WATCH_DOG,  LOW) ;
    }
    else {
        alive_GPIO=1;
        digitalWrite (LED_PROGRAM_ALIVE_WATCH_DOG,  HIGH) ;
    }

    if((ui->doubleSpinBox_pump3_thres->value() + pump3_hyst) > outside_temp){
        pump3_hyst = 1.0;
        digitalWrite (RELAY_INV_PUMP3,  LOW) ;
        ui->checkBox_pump3->setChecked(true);

    } else
    {
        pump3_hyst = 0.0;
        digitalWrite (RELAY_INV_PUMP3,  HIGH) ;
        ui->checkBox_pump3->setChecked(false);
    }
    bool checkbox_shunt2_auto_pool = ui->checkBox_shunt2_auto_pool->checkState();
    bool checkbox_shunt2_man_pool = ui->checkBox_shunt2_man_pool->checkState();
    bool checkbox_shunt2_fire = ui->checkBox_shunt2_fire->checkState();
    if(checkbox_shunt2_auto_pool == false && checkbox_shunt2_man_pool == false && checkbox_shunt2_fire == false)
    {
        ui->checkBox_shunt2_auto_pool->setChecked(true);
        checkbox_shunt2_auto_pool = ui->checkBox_shunt2_auto_pool->checkState();
    }



    solar_top_diff_ON = ui->doubleSpinBox_solar_diff_on->value();
    solar_top_diff_OFF = ui->doubleSpinBox_solar_diff_off->value();

     if(checkbox_shunt2_fire == true)
     {
 //       if(hot_w_temp_sensor < (((double)ui->spinBox_manual_hotwater->value()) - 10.0) || solar_exchanger < ((double)ui->spinBox_manual_hotwater->value()) ){
         if((solar_exchanger + 0.0) < ((double)ui->spinBox_manual_hotwater->value()) && fire_auto_off_timer == 0){

         //Check special case if fire run out of fuel and tap water drop then turn OFF pump 2 if solar also lower then tap water
            digitalWrite (RELAY_PUMP2,  LOW) ;
            digitalWrite (RELAY_INV_PUMP2,  HIGH) ;
            ui->checkBox_pump2->setChecked(false);
            emit shunt2_contr_ON(false);
            set_shunt2to_cw();
            printf("Fire may run out of fuel. stop PUMP2\n");
            ui->checkBox_shunt2_fire->setChecked(false);
            ui->checkBox_shunt2_man_pool->setChecked(false);
            ui->checkBox_shunt2_auto_pool->setChecked(false);
            checkbox_shunt2_auto_pool = ui->checkBox_shunt2_auto_pool->checkState();
            checkbox_shunt2_man_pool = ui->checkBox_shunt2_man_pool->checkState();
            checkbox_shunt2_fire = ui->checkBox_shunt2_fire->checkState();

         }
        else{
            //Normal fire have heat up the tap water. Then its OK to get hotwater to the radiator also
            digitalWrite (RELAY_PUMP2,  HIGH) ;
            digitalWrite (RELAY_INV_PUMP2,  LOW) ;
            ui->checkBox_pump2->setChecked(true);
            emit shunt2_contr_ON(true);
        }
    }
     else{
         emit shunt2_contr_ON(false);
     }
    if(checkbox_shunt2_man_pool == true){
        solar_to_pool();
    }
    if(checkbox_shunt2_auto_pool == true){
        double max_temp = ui->doubleSpinBox_max_water_top->value();
        double min_temp = ui->doubleSpinBox_min_water_top->value();
        if(hot_w_temp_sensor > max_temp){
            //Switch to pool heating
            solar2pool_state = true;
        }
        if(hot_w_temp_sensor < min_temp){
            //Switch to hot water heating
            solar2pool_state = false;
        }
        if(solar2pool_state == true){
            solar_to_pool();
        }else {
            solar_to_hotwater();
        }

    }
    printf("Time houer = %d\n", time.hour());
    //Extract Temp profile data from 0..23 sliders
    int temp_profile_int = 0;
    if(reinit_timer>0){
        reinit_timer--;
    }
    printf("reinit_timer = %d\n", reinit_timer);
    printf("Counter debug_reinit_low_temp_hot_w = %d\n", debug_reinit_low_temp_hot_w);
    //--------- Special case if hot water is above 60 C protect heatpump set in tap water mode ------------------
    high_temp_hot_w_th_b = (double)ui->spinBox_high_temp_hot_w_th->value();
    if(high_temp_hot_w_th_b < hot_w_temp_sensor)
    {
        ui->checkBox_hotwater_mode->setEnabled(false);
        bool hotwater_m = ui->checkBox_hotwater_mode->checkState();
        if(hotwater_m == true){
            //Don't set in hot water mode because hot water is hig temp (over +60C)
            ui->checkBox_hotwater_mode->setChecked(false);
            ui->checkBox_heater_mode->setChecked(true);//
        }
    }
    //-----------------------------------------------------------------------------------------------------------

    switch (time.hour()) {
    case(0):
        temp_profile_int = ui->verticalSlider_0->value();
        break;
    case(1):
        temp_profile_int = ui->verticalSlider_1->value();
        break;
    case(2):
        temp_profile_int = ui->verticalSlider_2->value();
        break;
    case(3):
        temp_profile_int = ui->verticalSlider_3->value();
        break;
    case(4):
        temp_profile_int = ui->verticalSlider_4->value();
        break;
    case(5):
        temp_profile_int = ui->verticalSlider_5->value();
        break;
    case(6):
        temp_profile_int = ui->verticalSlider_6->value();
        break;
    case(7):
        temp_profile_int = ui->verticalSlider_7->value();
        break;
    case(8):
        temp_profile_int = ui->verticalSlider_8->value();
        break;
    case(9):
        temp_profile_int = ui->verticalSlider_9->value();
        break;
    case(10):
        temp_profile_int = ui->verticalSlider_10->value();
        break;
    case(11):
        temp_profile_int = ui->verticalSlider_11->value();
        break;
    case(12):
        temp_profile_int = ui->verticalSlider_12->value();
        break;
    case(13):
        temp_profile_int = ui->verticalSlider_13->value();
        break;
    case(14):
        temp_profile_int = ui->verticalSlider_14->value();
        break;
    case(15):
        temp_profile_int = ui->verticalSlider_15->value();
        break;
    case(16):
        temp_profile_int = ui->verticalSlider_16->value();
        break;
    case(17):
        temp_profile_int = ui->verticalSlider_17->value();
        break;
    case(18):
        temp_profile_int = ui->verticalSlider_18->value();
        break;
    case(19):
        temp_profile_int = ui->verticalSlider_19->value();
        break;
    case(20):
        temp_profile_int = ui->verticalSlider_20->value();
        break;
    case(21):
        temp_profile_int = ui->verticalSlider_21->value();
        break;
    case(22):
        temp_profile_int = ui->verticalSlider_22->value();
        break;
    case(23):
        temp_profile_int = ui->verticalSlider_23->value();
        break;
    }
    temp_profile = (double)temp_profile_int * 0.1;
    ui->lcdNumber_day_profile_temp->display(QString::number(temp_profile));

    //
    bool checkbox_auto = ui->checkBox_auto->checkState();
    bool checkbox_radiator_mode = ui->checkBox_heater_mode->checkState();
    bool checkbox_tap_water_mode = ui->checkBox_hotwater_mode->checkState();
    bool checkbox_both_mode = ui->checkBox_hotwater_and_heater_mode->checkState();
    emit PID_forward(forward_signal);
    emit PID_feedback(inhouse_temp);
    emit PID_setpoint(temp_setp_with_profile);

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

                ui->checkBox_shunt2_fire->setEnabled(false);
                ui->checkBox_shunt2_fire->setChecked(false);


                emit controller_mode(CONTROLLER_MODE_RESET_PID);
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
                    man_mode_checkbox_update();
                    ui->checkBox_shunt2_fire->setEnabled(true);
                    if((outside_temp + hysteres_auto_off) > ui->doubleSpinBox_auto_off_outside->value() && (temperature_matrix[1] + hysteres_auto_off) > ui->spinBox_auto_off_actual->value() && heatpump_reply[REPLY_INDEX_3_ACTUAL_TEMP] > 20){
                        hysteres_auto_off = HYSTERESIS_LEVEL;
                        //Turn off radiator mode because outside is warm weather
                        printf("AUTO OFF RADIATOR mode\n");
                        emit controller_mode(CONTROLLER_MODE_PAUSE_PID);
                        if(heatpump_reply[REPLY_INDEX_7_u4] == REPLY_I7_hotwater_44)
                        {//Heatpump is now in Commanded tap water mode
                            if(heatpump_reply[REPLY_INDEX_4_SETP_TEMP] == ui->spinBox_manual_hotwater->value()){
                                heatpump_send[CMD_INDEX_0] = CMD_hotwater;//Stay in hot water mode
                            }
                            else{
                                heatpump_send[CMD_INDEX_0] = CMD_SET_TEMP;//set_temp
                                heatpump_send[SET_INDEX_1_SETP_TEMP] = ui->spinBox_manual_hotwater->value();
                            }
                        }
                        else
                        {
                            heatpump_send[CMD_INDEX_0] = CMD_hotwater;
                        }
                    }
                    else{
                        hysteres_auto_off = -HYSTERESIS_LEVEL;
                        if(heatpump_reply[REPLY_INDEX_6_u5] == REPLY_I6_Hot_555){//Heatpump is now in radiator mode
                            emit controller_mode(CONTROLLER_MODE_RUN_PID);
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
                            {//Heatpump is NOT in Commanded radiator mode, now heatpump is in commanded tap water mode or both mode
                                if(heatpump_reply[REPLY_INDEX_4_SETP_TEMP] != ui->spinBox_man_temp_hp->value()){
                                    heatpump_send[CMD_INDEX_0] = CMD_hot;
                                }
                            }
                        }
                        else{//Heatpump is NOT in radiator mode,  NOT REPLY_I6_Hot_555 mode
                            //Wait until heater have jump over to radiator mode when tap water is heated up
                            //We could send both mode also when waiting
                            emit controller_mode(CONTROLLER_MODE_PAUSE_PID);
                            heatpump_send[CMD_INDEX_0] = CMD_hot_hotwater;//both mode
                        }
                    }
                    //
                }
            }
 //*******
        }
        else if(heatpump_reply[REPLY_INDEX_8_u1] == REPLY_I8_11_OFF)
        {
            //OFF
            ui->label_pic->setPixmap(*OFF_pix);
            emit controller_mode(CONTROLLER_MODE_RESET_PID);
        }
        else{
            //ui->label_pic->setPixmap(*error_pix);
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
            //--------- Special case if hot water is above 60 C protect heatpump set in tap water mode ------------------
            if(high_temp_hot_w_th_b < hot_w_temp_sensor)
            {
                start_up = 8;//Jump over tap water mode because tap water is hot;
            }
            //-----------------------------------------------------------------------------------------------------------
            else{
            set_tap_water_mode();
            heatpump_send[CMD_INDEX_0] = CMD_hotwater;
            tick_cnt1++;
            if(tick_cnt1>4){
                start_up++;
            }
            }
            break;
        case(6):
            //--------- Special case if hot water is above 60 C protect heatpump set in tap water mode ------------------
            if(high_temp_hot_w_th_b < hot_w_temp_sensor)
            {
                start_up = 8;//Jump over tap water mode because tap water is hot;
            }
            //-----------------------------------------------------------------------------------------------------------
            else{
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
            }
            break;
        case(7):
            //--------- Special case if hot water is above 60 C protect heatpump set in tap water mode ------------------
            if(high_temp_hot_w_th_b < hot_w_temp_sensor)
            {
                start_up = 8;//Jump over tap water mode because tap water is hot;
            }
            //-----------------------------------------------------------------------------------------------------------
            else{
                if(heatpump_reply[REPLY_INDEX_4_SETP_TEMP] == ui->spinBox_manual_hotwater->value())
                {
                    //tap water setpoint reply is equal to Spinbox Send tap water setpoint
                    tick_cnt1++;
                    if(tick_cnt1>4){
                        start_up++;
                    }
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
                emit controller_mode(CONTROLLER_MODE_RESET_PID);
            }
            break;

        default:
            auto_mode_turn_on();
            emit_PID_parameters();
            emit controller_mode(CONTROLLER_MODE_RESET_PID);
            tick_cnt1 = 0;
            break;
        }

        //End start up
    }
    else {
        if(heatpump_reply[REPLY_INDEX_9_uError] == REPLY_I9_NETWORK_ERR){
            //NETWORK ERROR
            ui->label_pic->setPixmap(*error_pix);

        }
    }
    if(fire_auto_off_timer > 0){
        fire_auto_off_timer--;
    }
    ui->lineEdit_shunt2_timer_fire_auto_off->setText(QString::number((double)fire_auto_off_timer, 'f', 0));
}
void MainWindow::update_profile_lable(void)
{
    ui->label_profil_0->setNum(ui->verticalSlider_0->value() * 0.1);
    ui->label_profil_1->setNum(ui->verticalSlider_1->value() * 0.1);
    ui->label_profil_2->setNum(ui->verticalSlider_2->value() * 0.1);
    ui->label_profil_3->setNum(ui->verticalSlider_3->value() * 0.1);
    ui->label_profil_4->setNum(ui->verticalSlider_4->value() * 0.1);
    ui->label_profil_5->setNum(ui->verticalSlider_5->value() * 0.1);
    ui->label_profil_6->setNum(ui->verticalSlider_6->value() * 0.1);
    ui->label_profil_7->setNum(ui->verticalSlider_7->value() * 0.1);
    ui->label_profil_8->setNum(ui->verticalSlider_8->value() * 0.1);
    ui->label_profil_9->setNum(ui->verticalSlider_9->value() * 0.1);
    ui->label_profil_10->setNum(ui->verticalSlider_10->value() * 0.1);
    ui->label_profil_11->setNum(ui->verticalSlider_11->value() * 0.1);
    ui->label_profil_12->setNum(ui->verticalSlider_12->value() * 0.1);
    ui->label_profil_13->setNum(ui->verticalSlider_13->value() * 0.1);
    ui->label_profil_14->setNum(ui->verticalSlider_14->value() * 0.1);
    ui->label_profil_15->setNum(ui->verticalSlider_15->value() * 0.1);
    ui->label_profil_16->setNum(ui->verticalSlider_16->value() * 0.1);
    ui->label_profil_17->setNum(ui->verticalSlider_17->value() * 0.1);
    ui->label_profil_18->setNum(ui->verticalSlider_18->value() * 0.1);
    ui->label_profil_19->setNum(ui->verticalSlider_19->value() * 0.1);
    ui->label_profil_20->setNum(ui->verticalSlider_20->value() * 0.1);
    ui->label_profil_21->setNum(ui->verticalSlider_21->value() * 0.1);
    ui->label_profil_22->setNum(ui->verticalSlider_22->value() * 0.1);
    ui->label_profil_23->setNum(ui->verticalSlider_23->value() * 0.1);

}
void MainWindow::emit_PID_parameters(void)
{
    emit PID_p_p(ui->doubleSpinBox_pid_p->value());
    emit PID_p_i(ui->doubleSpinBox_pid_i->value());
    emit PID_p_d(ui->doubleSpinBox_pid_d->value());
    emit PID_p_cvu(ui->spinBox_pid_cvu->value());
    emit PID_p_cvl(ui->spinBox_pid_cvl->value());
    emit PID_p_tau_d(ui->spinBox_pid_d_tau->value());
    emit PID_p_tau_i(ui->spinBox_pid_reset_tau->value());
    emit PID_update_samp(ui->spinBox_pid_control_samp->value());
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
        bool auto_checkbox = ui->checkBox_auto->checkState();
        if(auto_checkbox == false){
            ui->checkBox_heater_mode->setEnabled(true);
            ui->checkBox_hotwater_mode->setEnabled(true);
            ui->checkBox_hotwater_and_heater_mode->setEnabled(true);
            man_mode_checkbox_update();
        }
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
    emit_PID_parameters();
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


void MainWindow::on_doubleSpinBox_offset_forward_valueChanged(double arg1)
{
    //PID_forward_offset = arg1;
    //emit_PID_parameters();
}


void MainWindow::PID_control_signal(double arg1)
{
    //New value from PID arrived
    ui->lineEdit_control_value->setText(QString::number(arg1, 'f', 3));
    if(auto_init_done == true){
        bool firecont = ui->checkBox_shunt2_fire->checkState();
        if(firecont == true){
            arg1 = ui->spinBox_pid_cvl->value();
        }
        ui->spinBox_man_temp_hp->setValue((int)arg1);
    }
}

void MainWindow::on_spinBox_pid_cvl_valueChanged(int arg1)
{
    emit_PID_parameters();
}

void MainWindow::on_spinBox_pid_cvu_valueChanged(int arg1)
{
    emit_PID_parameters();
}

void MainWindow::on_doubleSpinBox_pid_i_valueChanged(double arg1)
{
    emit_PID_parameters();
}

void MainWindow::on_doubleSpinBox_pid_d_valueChanged(double arg1)
{
    emit_PID_parameters();
}

void MainWindow::on_spinBox_pid_reset_tau_valueChanged(int arg1)
{
    emit_PID_parameters();
}

void MainWindow::on_spinBox_pid_d_tau_valueChanged(int arg1)
{
    emit_PID_parameters();
}

void MainWindow::on_spinBox_pid_control_samp_valueChanged(int arg1)
{
    emit_PID_parameters();
}

void MainWindow::on_doubleSpinBox_gain_forward_valueChanged(double arg1)
{
    emit_PID_parameters();
}


void MainWindow::on_verticalSlider_0_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_1_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_2_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_3_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_4_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_5_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_6_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_7_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_8_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_9_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_10_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_11_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_12_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_13_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_14_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_15_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_16_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_17_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_18_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_19_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_20_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_21_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_22_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_verticalSlider_23_valueChanged(int value)
{
    update_profile_lable();
}

void MainWindow::on_spinBox_auto_off_actual_valueChanged(int arg1)
{

}

void MainWindow::on_checkBox_shunt2_fire_clicked(bool checked)
{
    if(checked == true){
        //Set heatpum in automode to enable PID regulator
        ui->checkBox_auto->setChecked(true);
        auto_mode_turn_on();
        ui->checkBox_shunt2_man_pool->setChecked(false);
        ui->checkBox_shunt2_auto_pool->setChecked(false);

        fire_auto_off_timer = ui->spinBox_shunt2_auto_off->value();
//        emit shunt2_contr_ON(true);
    }
    else {
//        emit shunt2_contr_ON(false);
    }

}

void MainWindow::PID_control_instant_signal(double arg1)
{
    ui->lineEdit_control_value_raw->setText(QString::number(arg1, 'f', 3));
}

void MainWindow::on_checkBox_shunt2_man_pool_clicked(bool checked)
{
    if(checked == true){
        ui->checkBox_shunt2_auto_pool->setChecked(false);
        ui->checkBox_shunt2_fire->setChecked(false);
//        emit shunt2_contr_ON(false);
    }
}

void MainWindow::on_checkBox_shunt2_auto_pool_clicked(bool checked)
{
    if(checked == true){
        ui->checkBox_shunt2_man_pool->setChecked(false);
        ui->checkBox_shunt2_fire->setChecked(false);
//        emit shunt2_contr_ON(false);
    }
}

void MainWindow::on_doubleSpinBox_shunt2_gain_valueChanged(double arg1)
{
    emit shunt2_gain_par(arg1);
}

void MainWindow::on_doubleSpinBox_shunt2_hyst_valueChanged(double arg1)
{
    emit shunt2_hyst_par(arg1);
}

void MainWindow::indicator_shunt2_cw(bool arg1)
{
    ui->checkBox_shunt2_cw->setChecked(arg1);
}
void MainWindow::indicator_shunt2_ccw(bool arg1)
{
    ui->checkBox_shunt2_ccw->setChecked(arg1);
}



void MainWindow::on_pushButton_clicked()
{
    save_dialog_quest();
}
void MainWindow::save_dialog_quest(void){
    save_dialog save_dialog_obj;
    save_obj = new save_dialog;
    connect(save_obj, SIGNAL(user_save_settings(bool)), this, SLOT(user_save_settings(bool)));
    save_obj->exec();
    delete save_obj;

}
void MainWindow::user_save_settings(bool arg1)
{
    if(arg1==true){
        save_settings();
    }
}

void MainWindow::save_settings(void)
{
    printf("Save all settings\n");
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

    //QString y = QString::number((double)Mixer_inhouse_1, 10, 2);
    mySettings->setValue(QString("mySettings/Mixer_inhouse_1"), Mixer_inhouse_1);
    QString y = QString::number(ui->doubleSpinBox_gain_forward->value(), 10, 9);
    mySettings->setValue(QString("mySettings/PID_forward_gain"), y);
    y = QString::number(ui->doubleSpinBox_offset_forward->value(), 10, 9);
    mySettings->setValue(QString("mySettings/PID_forward_offset"), y);


    mySettings->setValue(QString("mySettings/PID_par_cvu"), ui->spinBox_pid_cvu->value());
    mySettings->setValue(QString("mySettings/PID_par_cvl"), ui->spinBox_pid_cvl->value());
    y = QString::number(ui->doubleSpinBox_pid_p->value(), 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_p"), y);
    y = QString::number(ui->doubleSpinBox_pid_i->value(), 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_i"), y);
    y = QString::number(ui->doubleSpinBox_pid_d->value(), 10, 9);
    mySettings->setValue(QString("mySettings/PID_par_d"), y);
    mySettings->setValue(QString("mySettings/PID_par_tau_i"), ui->spinBox_pid_reset_tau->value());
    mySettings->setValue(QString("mySettings/PID_par_tau_d"), ui->spinBox_pid_d_tau->value());
    y = QString::number(ui->doubleSpinBox_inhouse_setp->value(), 10, 9);
    mySettings->setValue(QString("mySettings/temp_setp_1"), y);
    mySettings->setValue(QString("mySettings/PID_update_strobe"), ui->spinBox_pid_control_samp->value());
    mySettings->setValue(QString("mySettings/high_temp_hot_w_th_b"), ui->spinBox_high_temp_hot_w_th->value());
    mySettings->setValue(QString("mySettings/hot_w_low_threshold_b"), ui->spinBox_hot_w_low_th->value());

    y = QString::number(ui->doubleSpinBox_auto_off_outside->value(), 10, 9);
    mySettings->setValue(QString("mySettings/auto_off_outside"), y);
    mySettings->setValue(QString("mySettings/auto_off_actual"), ui->spinBox_auto_off_actual->value());
    mySettings->setValue(QString("mySettings/shunt2_auto_off"), ui->spinBox_shunt2_auto_off->value());
    y = QString::number(ui->doubleSpinBox_solar_diff_on->value(), 10, 9);
    mySettings->setValue(QString("mySettings/solar_diff_on"), y);
    y = QString::number(ui->doubleSpinBox_solar_diff_off->value(), 10, 9);
    mySettings->setValue(QString("mySettings/solar_diff_off"), y);
    y = QString::number(ui->doubleSpinBox_max_water_top->value(), 10, 9);
    mySettings->setValue(QString("mySettings/max_water_top"), y);
    y = QString::number(ui->doubleSpinBox_min_water_top->value(), 10, 9);
    mySettings->setValue(QString("mySettings/min_water_top"), y);

    y = QString::number(ui->doubleSpinBox_pump3_thres->value(), 10, 9);
    mySettings->setValue(QString("mySettings/pump3_thres"), y);

    y = QString::number(ui->doubleSpinBox_shunt2_hyst->value(), 10, 9);
    mySettings->setValue(QString("mySettings/shunt2_hyst"), y);
    y = QString::number(ui->doubleSpinBox_shunt2_gain->value(), 10, 9);
    mySettings->setValue(QString("mySettings/shunt2_gain"), y);
    y = QString::number(ui->doubleSpinBox_pid_shunt2_cvu->value(), 10, 9);
    mySettings->setValue(QString("mySettings/shunt2_cvu_par"), y);
    y = QString::number(ui->doubleSpinBox_pid_shunt2_cvl->value(), 10, 9);
    mySettings->setValue(QString("mySettings/shunt2_cvl_par"), y);
    y = QString::number(ui->doubleSpinBox_pid_shunt2_i->value(), 10, 9);
    mySettings->setValue(QString("mySettings/shunt2_i_par"), y);
    y = QString::number(ui->doubleSpinBox_pid_shunt2_d->value(), 10, 9);
    mySettings->setValue(QString("mySettings/shunt2_d_par"), y);
    y = QString::number(ui->doubleSpinBox_pid_shunt2_tau_i->value(), 10, 9);
    mySettings->setValue(QString("mySettings/shunt2_tau_i"), y);
    y = QString::number(ui->doubleSpinBox_pid_shunt2_tau_d->value(), 10, 9);
    mySettings->setValue(QString("mySettings/shunt2_tau_d"), y);


    mySettings->setValue(QString("mySettings/day_houer_profile_0"), ui->verticalSlider_0->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_1"), ui->verticalSlider_1->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_2"), ui->verticalSlider_2->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_3"), ui->verticalSlider_3->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_4"), ui->verticalSlider_4->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_5"), ui->verticalSlider_5->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_6"), ui->verticalSlider_6->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_7"), ui->verticalSlider_7->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_8"), ui->verticalSlider_8->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_9"), ui->verticalSlider_9->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_10"), ui->verticalSlider_10->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_11"), ui->verticalSlider_11->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_12"), ui->verticalSlider_12->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_13"), ui->verticalSlider_13->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_14"), ui->verticalSlider_14->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_15"), ui->verticalSlider_15->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_16"), ui->verticalSlider_16->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_17"), ui->verticalSlider_17->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_18"), ui->verticalSlider_18->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_19"), ui->verticalSlider_19->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_20"), ui->verticalSlider_20->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_21"), ui->verticalSlider_21->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_22"), ui->verticalSlider_22->value());
    mySettings->setValue(QString("mySettings/day_houer_profile_23"), ui->verticalSlider_23->value());

    mySettings->sync();//save mySettings

}

void MainWindow::on_doubleSpinBox_pid_shunt2_cvu_valueChanged(double arg1)
{
    emit shunt2_cvu_par(arg1);
}

void MainWindow::on_doubleSpinBox_pid_shunt2_cvl_valueChanged(double arg1)
{
    emit shunt2_cvl_par(arg1);
}

void MainWindow::on_doubleSpinBox_pid_shunt2_i_valueChanged(double arg1)
{
    emit shunt2_i_par(arg1);
}

void MainWindow::on_doubleSpinBox_pid_shunt2_d_valueChanged(double arg1)
{
    emit shunt2_d_par(arg1);
}

void MainWindow::on_doubleSpinBox_pid_shunt2_tau_i_valueChanged(double arg1)
{
    emit shunt2_tau_i(arg1);
}

void MainWindow::on_doubleSpinBox_pid_shunt2_tau_d_valueChanged(double arg1)
{
    emit shunt2_tau_d(arg1);
}

void MainWindow::indicator_shunt2_d_part(double arg1)
{
    ui->lineEdit_shunt2_D_part->setText(QString::number(arg1, 'f', 7));

}
void MainWindow::indicator_shunt2_d_filt(double arg1)
{
    ui->lineEdit_shunt2_D_filt->setText(QString::number(arg1, 'f', 7));
}

void MainWindow::on_spinBox_shunt2_auto_off_valueChanged(int arg1)
{

}

void MainWindow::on_pushButton_2_clicked()
{
    //Pop up shunt 3 PID regulator for acc tank
    view_shunt3_regulator *view_shunt3_reg_inst;
    view_shunt3_reg_inst = new view_shunt3_regulator;
    view_shunt3_reg_inst->setWindowTitle("Shunt 3 regulator view");
    //Do all connect from view_shunt3.. object to shunt3 controller here ....
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_temperatur_setpoint(double)), shunt3obj1, SLOT(shunt3_temperatur_setpoint(double)));
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_cvu_par(double)), shunt3obj1, SLOT(shunt3_cvu_par(double)));
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_cvl_par(double)), shunt3obj1, SLOT(shunt3_cvl_par(double)));
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_gain_par(double)), shunt3obj1, SLOT(shunt3_gain_par(double)));
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_i_par(double)), shunt3obj1, SLOT(shunt3_i_parr(double)));
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_d_par(double)), shunt3obj1, SLOT(shunt3_d_par(double)));
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_tau_i(double)), shunt3obj1, SLOT(shunt3_tau_i(double)));
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_tau_d(double)), shunt3obj1, SLOT(shunt3_tau_d(double)));
    connect(view_shunt3_reg_inst, SIGNAL(shunt3_hyst_par(double)), shunt3obj1, SLOT(shunt3_hyst_par(double)));
    connect(shunt3obj1, SIGNAL(indicator_shunt3_cw(bool)), view_shunt3_reg_inst, SLOT(indicator_shunt3_cw(bool)));
    connect(shunt3obj1, SIGNAL(indicator_shunt3_ccw(bool)), view_shunt3_reg_inst, SLOT(indicator_shunt3_ccw(bool)));
    connect(shunt3obj1, SIGNAL(shunt3_D_filt(double)), view_shunt3_reg_inst, SLOT(shunt3_D_filt(double)));
    connect(shunt3obj1, SIGNAL(shunt3_D_part(double)), view_shunt3_reg_inst, SLOT(shunt3_D_part(double)));
    connect(shunt3obj1, SIGNAL(shunt3_I_part(double)), view_shunt3_reg_inst, SLOT(shunt3_I_part(double)));
    connect(shunt3obj1, SIGNAL(shunt3_P_part(double)), view_shunt3_reg_inst, SLOT(shunt3_P_part(double)));
    connect(shunt3obj1, SIGNAL(shunt3_control_value(double)), view_shunt3_reg_inst, SLOT(shunt3_control_value(double)));

    view_shunt3_reg_inst->mySettings = mySettings;
    view_shunt3_reg_inst->init_with_mySett();
    view_shunt3_reg_inst->exec();
    delete view_shunt3_reg_inst;


}



void MainWindow::on_checkBox_shunt3_heat_acc_clicked(bool checked)
{
    int on_off = 0;
    if(checked == true){
        on_off = 1;
    }else{
        on_off = 0;
    }
    emit shunt3_contr_ON(on_off);
    ui->label_debug->setText(QString::number(on_off));
}
