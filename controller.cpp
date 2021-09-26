#include "controller.h"
#include <stdio.h>
//This is the controller how handle the controller sampling rate and make the regultator controlling etc
#include <cmath> //Used for C11 not a number check
//bool isnan( float arg ); (since C++11)
//bool isnan( double arg ); (since C++11)
//bool isnan( long double arg ); (since C++11)
double check_and_clear_NaN(double arg1)
{
    if(std::isnan(arg1)){
        printf("ERROR NaN detected reset to 0.0\n");
        arg1 = 0.0;
    }
    return arg1;
}
double do_filter_constant(double samp_time, int tau)
{
    double filter_constant = 0.0;
    if(tau != 0)//Zero devision protection
    {
        filter_constant = samp_time / ((double)tau);
    }
    return filter_constant;
}
controller::controller(QObject *parent) : QObject(parent)
{
    sample_timer = new QTimer(this);
    sample_timer->start(1000);//ms
    sample_time=1.0;//sec
    connect(sample_timer, SIGNAL(timeout()), this, SLOT(timetick()));//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
    printf("Constructor controller\n");
    timetickcnt=0;
    PID_control_value=0.0;
    cont_mode= CONTROLLER_MODE_RESET_PID;
    integrator = 0.0;
    filtered_feedback= 0.0;
    prev_filt_feedback = 0.0;
    antiwindup_filter = 0.0;
    PID_d_filter_constant = 0.0;
    PID_res_i_filter_constant = 0.0;

}

void controller::controller_mode(int arg1)
{
    cont_mode = arg1;
}

void controller::timetick(void)//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
{
    emit controllertick();
    if(timetickcnt < PID_update_out_sec-1){
        timetickcnt++;
    }
    else {
        //Update output from PID
        timetickcnt = 0;
        emit PID_control_signal(PID_control_value);
    }

    //printf("Controller timetickcnt = %d\n", timetickcnt);
    integrator = check_and_clear_NaN(integrator);
    filtered_feedback = check_and_clear_NaN(filtered_feedback);
    antiwindup_filter = check_and_clear_NaN(antiwindup_filter);
    prev_filt_feedback = check_and_clear_NaN(prev_filt_feedback);
    double PID_error = 0.0;
    double i_part = 0.0;
    double p_part = 0.0;
    double d_part = 0.0;
    double cv_before_limit = 0.0;
    switch(cont_mode)
    {
    case(CONTROLLER_MODE_RESET_PID):
        integrator = 0.0;
        filtered_feedback= 0.0;
        antiwindup_filter = 0.0;
        break;

    case(CONTROLLER_MODE_RUN_PID):

        //*** Make PID error signal ***
        PID_error = PID_setp - PID_fb;
        //*****************************

        //*** P - part of the PID ***
        p_part = PID_error * PID_par_p;
        //***************************

        //*** I - part of the PID ***
        integrator = integrator + PID_error;
        i_part = integrator * PID_par_i;
        //***************************

        //*** D - part of the PID ***
        //Low pass filter the feedback for the make a filtered derivatives
        prev_filt_feedback = filtered_feedback;
        filtered_feedback = (PID_fb - filtered_feedback) * PID_d_filter_constant + filtered_feedback;//Low pass filter the feedback for Derivate
        d_part = (prev_filt_feedback - filtered_feedback) * PID_par_d;
        //***************************

        //*** Sum up and limit the PID output control ***
        cv_before_limit = p_part + i_part + d_part + PID_forw;
        if(cv_before_limit > (double)PID_par_cvu){
            PID_control_value = (double)PID_par_cvu;
        }
        else if (cv_before_limit < (double)PID_par_cvl) {
            PID_control_value = (double)PID_par_cvl;
        }
        else {
            PID_control_value = cv_before_limit;
        }
        //************************************************

        //*** Integrator Antiwindup - part of the PID ***
        antiwindup_filter = (PID_control_value - cv_before_limit) * PID_res_i_filter_constant;
        integrator = integrator + antiwindup_filter;
        //***********************************************

        printf("**************************\n");
        printf("PID_error =%f\n", PID_error);
        printf("integrator = %f\n", integrator);
        printf("antiwindup_filter =%f\n", antiwindup_filter);
        printf("p_part =%f\n", p_part);
        printf("i_part =%f\n", i_part);
        printf("d_part =%f\n", d_part);
        printf("cv_before_limit =%f\n", cv_before_limit);
        printf("PID_forw =%f\n", PID_forw);

        break;

    case(CONTROLLER_MODE_PAUSE_PID):
        //Just pause PID calculation, do nothing
        break;
    }
}

void controller::PID_feedback(double arg1)
{
    PID_fb = arg1;
  //  printf("PID_fb = %f\n", PID_fb);
}
void controller::PID_setpoint(double arg1)
{
    PID_setp = arg1;
  //  printf("PID_setp = %f\n", PID_setp);
}
void controller::PID_forward(double arg1)
{
    PID_forw = arg1;
  //  printf("PID_forw = %f\n", PID_forw);
}

void controller::PID_p_cvu(int arg1)
{
    PID_par_cvu = arg1;
    printf("PID_par_cvu = %d\n", PID_par_cvu);
}
void controller::PID_p_cvl(int arg1)
{
    PID_par_cvl = arg1;
    printf("PID_par_cvl = %d\n", PID_par_cvl);
}
void controller::PID_p_p(double arg1)
{
    PID_par_p = arg1;
}
void controller::PID_p_i(double arg1)
{
    PID_par_i = arg1;
}
void controller::PID_p_d(double arg1)
{
    PID_par_d = arg1;
}
void controller::PID_p_tau_i(int arg1)
{
    PID_par_tau_i = arg1;
    PID_res_i_filter_constant = do_filter_constant(sample_time, PID_par_tau_i);
}
void controller::PID_p_tau_d(int arg1)
{
    PID_par_tau_d = arg1;
    PID_d_filter_constant = do_filter_constant(sample_time, PID_par_tau_d);
}

void controller::PID_update_samp(int arg1)
{
    PID_update_out_sec = arg1;
}

