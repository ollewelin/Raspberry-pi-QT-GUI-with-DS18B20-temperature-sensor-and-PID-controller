#ifndef SHUNT3_CONTROLLER_H
#define SHUNT3_CONTROLLER_H

#include <QObject>
#include <QTimer>
#include "generic_pid_controller.h"
#include <QSettings>

class shunt3_controller : public QObject
{
    Q_OBJECT
public:
    explicit shunt3_controller(QObject *parent = nullptr);

    QSettings *mySettings;
    void init_with_mySett(void);

signals:
    void indicator_shunt3_cw(bool);
    void indicator_shunt3_ccw(bool);
    void shunt3_D_filt(double);
    void shunt3_D_part(double);
    void shunt3_I_part(double);
    void shunt3_P_part(double);
    void shunt3_control_value(double);


public slots:
    void timetick(void);
    void temperature_fb(double);
    void shunt3_temperatur_setpoint(double);
    void shunt3_contr_ON(int);
    void shunt3_cvu_par(double);
    void shunt3_cvl_par(double);
    void shunt3_gain_par(double);
    void shunt3_i_par(double);
    void shunt3_d_par(double);
    void shunt3_tau_i(double);
    void shunt3_tau_d(double);
    void shunt3_hyst_par(double);

private:
    QTimer *sample_timer;//timer driver for controller sampling rate
    int timetickcnt;
    //double sample_time;
    int shunt3_cnt;
    int shunt3_measure_time;
    int shunt3_contr_ON_OFF;
  //  double PID_setpoint;
    double shunt3_gain;
    double shunt3_hysteresis;
    double temp_sensor;
    int shunt_drive_state;
    int pre_shunt_drive_state;
    void both_off(void);
    generic_pid_controller* pid_obj;
    int print_cnt;

};

#endif // SHUNT3_CONTROLLER_H
