#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>

#define CONTROLLER_MODE_RESET_PID 0
#define CONTROLLER_MODE_RUN_PID 1
#define CONTROLLER_MODE_PAUSE_PID 2

class controller : public QObject
{
    Q_OBJECT
public:
    explicit controller(QObject *parent = nullptr);

signals:
    void controllertick(void);
    void PID_control_signal(double);
public slots:
    void timetick(void);
    void PID_feedback(double);
    void PID_setpoint(double);
    void PID_forward(double);
    void PID_p_cvu(int);
    void PID_p_cvl(int);
    void PID_p_p(double);
    void PID_p_i(double);
    void PID_p_d(double);
    void PID_p_tau_i(int);
    void PID_p_tau_d(int);
    void PID_update_samp(int);
    void controller_mode(int);


private:
    QTimer *sample_timer;//timer driver for controller sampling rate
    int timetickcnt;

    double PID_fb;
    double PID_setp;
    double PID_forw;
    int PID_par_cvu;
    int PID_par_cvl;
    double PID_par_p;
    double PID_par_i;
    double PID_par_d;
    int PID_par_tau_i;
    int PID_par_tau_d;
    double PID_control_value;
    int PID_update_out_sec;
    double sample_time;
    int cont_mode;

    //Internal
    double PID_d_filter_constant;
    double PID_res_i_filter_constant;

    //Internal memory signals in the PID regulator
    double integrator;
    double filtered_feedback;
    double prev_filt_feedback;
    double antiwindup_filter;
    //
};

#endif // CONTROLLER_H
