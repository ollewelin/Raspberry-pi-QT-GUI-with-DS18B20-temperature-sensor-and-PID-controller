#include "generic_pid_controller.h"
#include <stdio.h>
#include <cmath> //Used for C11 not a number check
//bool isnan( float arg ); (since C++11)
//bool isnan( double arg ); (since C++11)
//bool isnan( long double arg ); (since C++11)

//#define USE_PRINTOUT
double generic_pid_controller::check_and_clear_NaN(double arg1)
{
    if(std::isnan(arg1)){
        printf("ERROR NaN detected reset to 0.0\n");
        arg1 = 0.0;
    }
    return arg1;
}
double generic_pid_controller::do_filter_constant(double samp_time, double tau)
{
    double filter_constant = 0.0;
    if(tau != 0.0)//Zero devision protection
    {
        filter_constant = samp_time / tau;
    }
    return filter_constant;
}
#define CONTROLLER_MODE_RESET_PID 0
#define CONTROLLER_MODE_RUN_PID 1
#define CONTROLLER_MODE_PAUSE_PID 2

generic_pid_controller::generic_pid_controller()
{
    PID_control_value=0.0;
    cont_mode= CONTROLLER_MODE_RESET_PID;
    integrator = 0.0;
    filtered_feedback= 0.0;
    prev_filt_feedback = 0.0;
    antiwindup_filter = 0.0;
    PID_d_filter_constant = 0.0;
    PID_res_i_filter_constant = 0.0;
    sample_time = 0.0;
    d_part = 0.0;

}

void generic_pid_controller::run1sample(void)
{
    PID_res_i_filter_constant = do_filter_constant(sample_time, PID_par_tau_i);
    PID_d_filter_constant = do_filter_constant(sample_time, PID_par_tau_d);
//    printf("PID_d_filter_constant = %f\n", (float)PID_d_filter_constant);
//    printf("sample_time = %f\n", (float)sample_time);
//    printf("PID_par_tau_d = %f\n", (float)PID_par_tau_d);

    integrator = check_and_clear_NaN(integrator);
    filtered_feedback = check_and_clear_NaN(filtered_feedback);
    antiwindup_filter = check_and_clear_NaN(antiwindup_filter);
   // prev_filt_feedback = check_and_clear_NaN(prev_filt_feedback);
   // double PID_error = 0.0;
    double i_part = 0.0;
    double p_part = 0.0;
    //double d_part = 0.0;
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
        if(PID_par_i != 0.0){
            integrator = integrator + PID_error;
        }
        i_part = integrator * PID_par_i;
        //***************************

        //*** D - part of the PID ***
        //Low pass filter the feedback for the make a filtered derivatives
        prev_filt_feedback = filtered_feedback;
        filtered_feedback = (PID_fb - filtered_feedback) * PID_d_filter_constant + filtered_feedback;//Low pass filter the feedback for Derivate
        d_part = (prev_filt_feedback - filtered_feedback) * PID_par_d;
        //***************************

        //*** Sum up and limit the PID output control ***
        cv_before_limit = p_part + i_part + d_part;

        if(cv_before_limit > PID_par_cvu){
            PID_control_value = PID_par_cvu;
        }
        else if (cv_before_limit < PID_par_cvl) {
            PID_control_value = PID_par_cvl;
        }
        else {
            PID_control_value = cv_before_limit;
        }
        //************************************************

        //*** Integrator Antiwindup - part of the PID ***
        antiwindup_filter = (PID_control_value - cv_before_limit) * PID_res_i_filter_constant;
        integrator = integrator + antiwindup_filter;
        //***********************************************
#ifdef USE_PRINTOUT
        printf("********** Generic PID *********\n");
        printf("PID_error =%f\n", PID_error);
        printf("integrator = %f\n", integrator);
        printf("antiwindup_filter =%f\n", antiwindup_filter);
        printf("p_part =%f\n", p_part);
        printf("i_part =%f\n", i_part);
        printf("d_part =%f\n", d_part);
        printf("cv_before_limit =%f\n", cv_before_limit);
#endif
        break;

    case(CONTROLLER_MODE_PAUSE_PID):
        //Just pause PID calculation, do nothing
        break;
    }

}

generic_pid_controller::~generic_pid_controller()
{

}
double generic_pid_controller::get_d_filt_fb(void)
{
    return filtered_feedback;
}
double generic_pid_controller::get_antiwindup_filt(void)
{
    return antiwindup_filter;
}
double generic_pid_controller::get_d_part(void)
{
    return d_part;
}


double generic_pid_controller::get_integrator(void)
{
    return integrator;
}
