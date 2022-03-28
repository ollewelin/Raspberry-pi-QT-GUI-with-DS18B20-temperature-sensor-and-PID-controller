#ifndef GENERIC_PID_CONTROLLER_H
#define GENERIC_PID_CONTROLLER_H


class generic_pid_controller
{
public:
    generic_pid_controller();
    ~generic_pid_controller();
    void run1sample(void);
    double check_and_clear_NaN(double arg1);
    double do_filter_constant(double samp_time, double tau);

    double PID_fb;
    double PID_setp;
    double PID_error;
    double PID_par_cvu;
    double PID_par_cvl;
    double PID_par_p;
    double PID_par_i;
    double PID_par_d;
    double PID_par_tau_i;
    double PID_par_tau_d;
    double PID_control_value;
    //int PID_update_out_sec;
    double sample_time;
    int cont_mode;

    double get_d_filt_fb(void);
    double get_antiwindup_filt(void);
    double get_integrator(void);
    double get_d_part(void);
    double get_i_part(void);
    double get_p_part(void);

private:
    //Internal
    double PID_d_filter_constant;
    double PID_res_i_filter_constant;

    //Internal memory signals in the PID regulator
    double integrator;
    double filtered_feedback;
    double prev_filt_feedback;
    double antiwindup_filter;
    double d_part;
    double i_part;
    double p_part;
    double cv_before_limit;

    //
};

#endif // GENERIC_PID_CONTROLLER_H
