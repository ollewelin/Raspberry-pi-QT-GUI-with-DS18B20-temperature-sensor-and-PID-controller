#ifndef SHUNT2_CONTROLLER_H
#define SHUNT2_CONTROLLER_H

#include <QObject>
#include <QTimer>
#include "generic_pid_controller.h"

class shunt2_controller : public QObject
{
    Q_OBJECT
public:
    explicit shunt2_controller(QObject *parent = nullptr);

signals:
    void indicator_shunt2_cw(bool);
    void indicator_shunt2_ccw(bool);

public slots:
    void timetick(void);
    void PID_control_instant_signal(double);//No down sample
    void radiator_temp2(double);
    void shunt2_contr_ON(int);
    void shunt2_gain_par(double);
    void shunt2_hyst_par(double);
private:
    QTimer *sample_timer;//timer driver for controller sampling rate
    int timetickcnt;
    //double sample_time;
    int shunt2_cnt;
    int shunt2_measure_time;
    int blink;
    int shunt2_contr_ON_OFF;
    double PID_control_ins_sig;
    double shunt2_gain;
    double shunt2_hysteresis;
    double radiator_temp_sensor2;
    int shunt_drive_state;
    int pre_shunt_drive_state;
    void both_off(void);
    generic_pid_controller* pid_obj;
};

#endif // SHUNT2_CONTROLLER_H
