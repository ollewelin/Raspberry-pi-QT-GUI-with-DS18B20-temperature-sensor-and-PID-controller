#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
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
    void PID_p_cvu(int);
    void PID_p_cvl(int);
    void PID_p_p(double);
    void PID_p_i(double);
    void PID_p_d(double);
    void PID_p_tau_i(int);
    void PID_p_tau_d(int);
    void PID_update_samp(int);


private:
    QTimer *sample_timer;//timer driver for controller sampling rate
    int timetickcnt;

    double PID_fb;
    double PID_setp;
    int PID_par_cvu;
    int PID_par_cvl;
    double PID_par_p;
    double PID_par_i;
    double PID_par_d;
    int PID_par_tau_i;
    int PID_par_tau_d;
    double PID_control_value;
    int PID_update_out_sec;

};

#endif // CONTROLLER_H
