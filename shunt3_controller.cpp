#include "shunt3_controller.h"
#include <wiringPi.h>
#include "gpio_pin_map.h"
#include "shunt_define.h"
shunt3_controller::shunt3_controller(QObject *parent) : QObject(parent)
{
    sample_timer = new QTimer(this);
    const double ms_time = 100.0;
    sample_timer->start((int)ms_time);//ms
   // sample_time=0.1;//sec
    connect(sample_timer, SIGNAL(timeout()), this, SLOT(timetick()));//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
    printf("Constructor shunt3_controller\n");
    digitalWrite (RELAY_SHUNT3_CCW,  LOW) ;
    digitalWrite (RELAY_SHUNT3_CW,  HIGH) ;
    shunt_drive_state = SHUNT_BOTH_OFF;
    pre_shunt_drive_state = SHUNT_CW_OFF;
    shunt3_measure_time = SHUNT_FULL_TURN_TIME / 2;

    pid_obj = new generic_pid_controller;
    pid_obj->PID_fb = 0.0;
    pid_obj->PID_setp = 0.0;
    pid_obj->PID_par_d = 0.0;
    pid_obj->PID_par_i = 0.0;
    pid_obj->PID_par_p = 0.0;
    pid_obj->PID_par_cvl = 0.0;
    pid_obj->PID_par_cvu = 0.0;
    pid_obj->PID_par_tau_d = 0.0;
    pid_obj->PID_par_tau_i = 0.0;
    pid_obj->cont_mode = CONTROLLER_MODE_RUN_PID;
    pid_obj->sample_time = 1.0/(ms_time/1000.0);
    print_cnt =0;

}

void shunt3_controller::init_with_mySett(void)
{
    pid_obj->PID_par_p = (mySettings->value("mySettings/shunt3_gain", "").toDouble());
    shunt3_hysteresis = (mySettings->value("mySettings/shunt3_hyst", "").toDouble());
    pid_obj->PID_par_cvu = (mySettings->value("mySettings/shunt3_cvu_par", "").toDouble());
    pid_obj->PID_par_cvl = (mySettings->value("mySettings/shunt3_cvl_par", "").toDouble());
    pid_obj->PID_par_i = (mySettings->value("mySettings/shunt3_i_par", "").toDouble());
    pid_obj->PID_par_d = (mySettings->value("mySettings/shunt3_d_par", "").toDouble());
    pid_obj->PID_par_tau_i = (mySettings->value("mySettings/shunt3_tau_i", "").toDouble());
    pid_obj->PID_par_tau_d = (mySettings->value("mySettings/shunt3_tau_d", "").toDouble());
    pid_obj->PID_setp = (mySettings->value("mySettings/shunt3_temperatur_setpoint", "").toDouble());
}


void shunt3_controller::both_off(void)
{
    digitalWrite (RELAY_SHUNT3_CCW,  LOW) ;
    digitalWrite (RELAY_SHUNT3_CW,  LOW) ;
    emit indicator_shunt3_cw(false);
    emit indicator_shunt3_ccw(false);
}
void shunt3_controller::timetick(void)//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
{

    if(shunt3_contr_ON_OFF == 1)
    {
        pid_obj->cont_mode = CONTROLLER_MODE_RUN_PID;
        if(shunt3_cnt<100){
            shunt3_cnt++;
        }
        else {
            shunt3_cnt=0;
          //  printf("clear shunt3_cnt\n");
        }

       // pid_obj->PID_setp = 65.0;
        pid_obj->PID_fb = temp_sensor;
        pid_obj->run1sample();
        double shunt3_cv = pid_obj->PID_control_value;
        double temperature_error = pid_obj->PID_error;

        //Emit data for view window
        emit shunt3_D_filt(pid_obj->get_d_filt_fb());
        emit shunt3_D_part(pid_obj->get_d_part());
        emit shunt3_I_part(pid_obj->get_i_part());
        emit shunt3_P_part(pid_obj->get_p_part());
        emit shunt3_control_value(shunt3_cv);
        //==========================


        int shunt3_cv_int = (int)shunt3_cv;

        if(temperature_error > shunt3_hysteresis || temperature_error < -shunt3_hysteresis){
            //printf("shunt3_cnt = %d\n",shunt3_cnt);
            if(shunt3_cv_int < 0){
                //Turn down temperature with shunt2 by turn cw direction

                if((-shunt3_cv_int) > shunt3_cnt){
                    //Drive the shunt motor
                    if(shunt_drive_state == SHUNT_CCW_ON){
                        shunt_drive_state = SHUNT_CCW_OFF;//Make one dead time cycle prevent both motor dirve short time if flipped
                    }
                    else{
                        shunt_drive_state = SHUNT_CW_ON;
                    }
                }
                else {
                    //Stop shunt motor
                    shunt_drive_state = SHUNT_CW_OFF;
                }
            }else
            {
                //Turn up temperature with shunt2 by turn ccw direction
                if(shunt3_cv_int > shunt3_cnt){
                    //Drive the shunt motor
                    if(shunt_drive_state == SHUNT_CW_ON){
                        shunt_drive_state = SHUNT_CW_OFF;//Make one dead time cycle prevent both motor dirve short time if flipped
                    }
                    else{
                        shunt_drive_state = SHUNT_CCW_ON;
                    }
                }
                else {
                    //Stop shunt motor
                    shunt_drive_state = SHUNT_CCW_OFF;
                }
            }
        }
        else {
            shunt_drive_state = SHUNT_BOTH_OFF;
        }

        bool stop_shunt_drive = true;
        if(shunt3_measure_time > 0 && shunt_drive_state == SHUNT_CW_ON){
            shunt3_measure_time--;
            stop_shunt_drive = false;
        }
        if(shunt3_measure_time < SHUNT_FULL_TURN_TIME && shunt_drive_state == SHUNT_CCW_ON){
            shunt3_measure_time++;
            stop_shunt_drive = false;
        }
        if(stop_shunt_drive == true)
        {
            shunt_drive_state = SHUNT_BOTH_OFF;
        }
        //printf("shunt3_measure_time = %d\n", shunt3_measure_time);
        if(pre_shunt_drive_state != shunt_drive_state){
            //change relay states
            switch (shunt_drive_state) {
            case(SHUNT_BOTH_OFF):
                both_off();
                break;
            case(SHUNT_CW_ON):
                digitalWrite (RELAY_SHUNT3_CCW,  LOW) ;
                emit indicator_shunt3_ccw(false);
                digitalWrite (RELAY_SHUNT3_CW,  HIGH) ;
                emit indicator_shunt3_cw(true);
                break;
            case(SHUNT_CW_OFF):
                both_off();
                break;
            case(SHUNT_CCW_ON):
                digitalWrite (RELAY_SHUNT3_CCW,  HIGH) ;
                emit indicator_shunt3_ccw(true);
                digitalWrite (RELAY_SHUNT3_CW,  LOW) ;
                emit indicator_shunt3_cw(false);
                break;
            case(SHUNT_CCW_OFF):
                both_off();
                break;
            }
        }
        pre_shunt_drive_state = shunt_drive_state;
    }
    else{
        pid_obj->cont_mode =  CONTROLLER_MODE_RESET_PID;
        digitalWrite (RELAY_SHUNT3_CCW,  HIGH) ;
        emit indicator_shunt3_ccw(true);
        digitalWrite (RELAY_SHUNT3_CW,  LOW) ;
        emit indicator_shunt3_cw(false);

    }
}

void shunt3_controller::shunt3_temperatur_setpoint(double arg1)
{
    pid_obj->PID_setp = arg1;
}


void shunt3_controller::shunt3_contr_ON(int ON_OFF)
{
    shunt3_contr_ON_OFF = ON_OFF;
}

void shunt3_controller::shunt3_gain_par(double arg1)
{
    shunt3_gain = arg1;
    pid_obj->PID_par_p = arg1;
}
void shunt3_controller::shunt3_hyst_par(double arg1)
{
    shunt3_hysteresis = arg1;
}

void shunt3_controller::temperature_fb(double arg1)
{
    temp_sensor = arg1;
}

void shunt3_controller::shunt3_cvu_par(double arg1)
{
    pid_obj->PID_par_cvu = arg1;
}
void shunt3_controller::shunt3_cvl_par(double arg1)
{
    pid_obj->PID_par_cvl = arg1;
}
void shunt3_controller::shunt3_i_par(double arg1)
{
    pid_obj->PID_par_i = arg1;
}
void shunt3_controller::shunt3_d_par(double arg1)
{
    pid_obj->PID_par_d = arg1;
}
void shunt3_controller::shunt3_tau_i(double arg1)
{
    pid_obj->PID_par_tau_i = arg1;
}
void shunt3_controller::shunt3_tau_d(double arg1)
{
    pid_obj->PID_par_tau_d = arg1;
}
