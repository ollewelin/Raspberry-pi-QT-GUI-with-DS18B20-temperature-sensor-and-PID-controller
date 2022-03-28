#include "shunt2_controller.h"
#include <wiringPi.h>
#include "gpio_pin_map.h"
#include "shunt_define.h"



shunt2_controller::shunt2_controller(QObject *parent) : QObject(parent)
{
    sample_timer = new QTimer(this);
    const double ms_time = 100.0;
    sample_timer->start((int)ms_time);//ms
   // sample_time=0.1;//sec
    connect(sample_timer, SIGNAL(timeout()), this, SLOT(timetick()));//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
    printf("Constructor shunt2_controller\n");
    digitalWrite (LED_SHUNT2_ADJUST_BY_REGULATOR_2,  HIGH) ;
    blink=1;
    digitalWrite (RELAY_SHUNT2_CCW,  LOW) ;
    digitalWrite (RELAY_SHUNT2_CW,  HIGH) ;
    shunt_drive_state = SHUNT_BOTH_OFF;
    pre_shunt_drive_state = SHUNT_CW_OFF;
    shunt2_measure_time = SHUNT_FULL_TURN_TIME / 2;

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
void shunt2_controller::both_off(void)
{
    digitalWrite (RELAY_SHUNT2_CCW,  LOW) ;
    digitalWrite (RELAY_SHUNT2_CW,  LOW) ;
    emit indicator_shunt2_cw(false);
    emit indicator_shunt2_ccw(false);
}
void shunt2_controller::timetick(void)//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
{

    if(shunt2_contr_ON_OFF == 1)
    {
        pid_obj->cont_mode =  CONTROLLER_MODE_RUN_PID;
        if(shunt2_cnt<100){
            shunt2_cnt++;
        }
        else {
            shunt2_cnt=0;
          //  printf("clear shunt2_cnt\n");
        }

        //checkbox on_checkBox_shunt2_man_pool_clicked == true
        //Run the control of the shunt2 motor at 100ms sample rate. Fire
    //    double temperature_error = PID_control_ins_sig - radiator_temp_sensor2;
    //    double shunt2_cv = temperature_error * shunt2_gain * 10.0;
        pid_obj->PID_setp = PID_control_ins_sig;
        pid_obj->PID_fb = radiator_temp_sensor2;
        pid_obj->run1sample();

        double temperature_error = pid_obj->PID_error;
        double shunt2_cv = pid_obj->PID_control_value;

        int shunt2_cv_int = (int)shunt2_cv;
        if(temperature_error > shunt2_hysteresis || temperature_error < -shunt2_hysteresis){
            //printf("shunt2_cnt = %d\n",shunt2_cnt);
            if(shunt2_cv_int < 0){
                //Turn down temperature with shunt2 by turn cw direction

                if((-shunt2_cv_int) > shunt2_cnt){
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
                if(shunt2_cv_int > shunt2_cnt){
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

            if(shunt2_cnt==0){
               // printf("Inside temperature range don't turning the shunt\n");
            }
        }
        if(shunt2_cnt==0){

//            printf("PID_control_ins_sig = %f\n", (float)PID_control_ins_sig);
//            printf("radiator_temp_sensor2 = %f\n", (float)radiator_temp_sensor2);
//            printf("temperature_error = %f\n", (float)temperature_error);
//            printf("shunt2_cv = %f\n", (float)shunt2_cv);
//            printf("shunt2_cv_int = %d\n", shunt2_cv_int);
//            printf("****************************************");
        }
        if(print_cnt < 10){
            print_cnt++;
        }
        else {
            emit indicator_shunt2_d_filt(pid_obj->get_d_filt_fb());
            emit indicator_shunt2_d_part(pid_obj->get_d_part());
           print_cnt =0;
            printf("======== SHUNT 2 PID status =========================\n");
            printf("PID_control_ins_sig = %f\n", (float)PID_control_ins_sig);
            printf("radiator_temp_sensor2 = %f\n", (float)radiator_temp_sensor2);
            printf("temperature_error = %f\n", (float)temperature_error);
            printf("D filtered feedback = %f\n", (float)pid_obj->get_d_filt_fb());
            printf("Integrator = %f\n", (float)pid_obj->get_integrator());
            printf("Antiwindup filter = %f\n", (float)pid_obj->get_antiwindup_filt());
            printf("shunt2_cv = %f\n", (float)shunt2_cv);
            printf("D part =%f\n", pid_obj->get_d_part());
            printf("shunt2_cv_int = %d\n", shunt2_cv_int);
            printf("======== xxxxxxxxxxxxxxxxxx =========================\n");

        }


        bool stop_shunt_drive = true;
        if(shunt2_measure_time > 0 && shunt_drive_state == SHUNT_CW_ON){
            shunt2_measure_time--;
            stop_shunt_drive = false;
        }
        if(shunt2_measure_time < SHUNT_FULL_TURN_TIME && shunt_drive_state == SHUNT_CCW_ON){
            shunt2_measure_time++;
            stop_shunt_drive = false;
        }
        if(stop_shunt_drive == true)
        {
            shunt_drive_state = SHUNT_BOTH_OFF;
            digitalWrite (LED_SHUNT2_ADJUST_BY_REGULATOR_2,  LOW) ;
        }else {
            if(blink>0){
                blink=0;
                digitalWrite (LED_SHUNT2_ADJUST_BY_REGULATOR_2,  HIGH) ;
            }
            else{
                blink=1;
                digitalWrite (LED_SHUNT2_ADJUST_BY_REGULATOR_2,  LOW) ;
            }
        }
        //printf("shunt2_measure_time = %d\n", shunt2_measure_time);
        if(pre_shunt_drive_state != shunt_drive_state){
            //change relay states
            switch (shunt_drive_state) {
            case(SHUNT_BOTH_OFF):
                both_off();
                break;
            case(SHUNT_CW_ON):
                digitalWrite (RELAY_SHUNT2_CCW,  LOW) ;
                emit indicator_shunt2_ccw(false);
                digitalWrite (RELAY_SHUNT2_CW,  HIGH) ;
                emit indicator_shunt2_cw(true);
                break;
            case(SHUNT_CW_OFF):
                both_off();
                break;
            case(SHUNT_CCW_ON):
                if(digitalRead(INPUT_SHUNT2_HALF_WAY_HALL_SWITCH) == HIGH){
                    //The Shunt is less then half full thottle then it is OK to increase
                    digitalWrite (RELAY_SHUNT2_CCW,  HIGH) ;
                }
                else {
                    //Stop increase shunt2 if HALL sensor have indicate more then 50% throttle
                    digitalWrite (RELAY_SHUNT2_CCW,  LOW) ;
                }
                emit indicator_shunt2_ccw(true);
                digitalWrite (RELAY_SHUNT2_CW,  LOW) ;
                emit indicator_shunt2_cw(false);
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
    }
}

void shunt2_controller::PID_control_instant_signal(double arg1)
{
    printf("PID_control_instant_signal = %f\n", (float)arg1);
    PID_control_ins_sig = arg1;
}

void shunt2_controller::shunt2_contr_ON(int ON_OFF)
{
    shunt2_contr_ON_OFF = ON_OFF;
}

void shunt2_controller::shunt2_gain_par(double arg1)
{
    shunt2_gain = arg1;
    pid_obj->PID_par_p = arg1;
}
void shunt2_controller::shunt2_hyst_par(double arg1)
{
    shunt2_hysteresis = arg1;
}

void shunt2_controller::radiator_temp2(double arg1)
{
    radiator_temp_sensor2 = arg1;
}

void shunt2_controller::shunt2_cvu_par(double arg1)
{
    pid_obj->PID_par_cvu = arg1;
}
void shunt2_controller::shunt2_cvl_par(double arg1)
{
    pid_obj->PID_par_cvl = arg1;
}
void shunt2_controller::shunt2_i_par(double arg1)
{
    pid_obj->PID_par_i = arg1;
}
void shunt2_controller::shunt2_d_par(double arg1)
{
    pid_obj->PID_par_d = arg1;
}
void shunt2_controller::shunt2_tau_i(double arg1)
{
    pid_obj->PID_par_tau_i = arg1;
}
void shunt2_controller::shunt2_tau_d(double arg1)
{
    pid_obj->PID_par_tau_d = arg1;
}
