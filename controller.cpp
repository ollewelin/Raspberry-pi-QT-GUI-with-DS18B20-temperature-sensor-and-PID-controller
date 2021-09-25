#include "controller.h"
#include <stdio.h>
//This is the controller how handle the controller sampling rate and make the regultator controlling etc

controller::controller(QObject *parent) : QObject(parent)
{
    sample_timer = new QTimer(this);
    sample_timer->start(1000);//ms
    connect(sample_timer, SIGNAL(timeout()), this, SLOT(timetick()));//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
    printf("Constructor controller\n");
    timetickcnt=0;

}

void controller::timetick(void)//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
{
    emit controllertick();
    timetickcnt++;
    printf("Controller timetickcnt = %d\n", timetickcnt);
}


void controller::PID_feedback(double arg1)
{
    PID_fb = arg1;
}
void controller::PID_setpoint(double arg1)
{
    PID_setp = arg1;
}
void controller::PID_p_cvu(int arg1)
{
    PID_par_cvu = arg1;
}
void controller::PID_p_cvl(int arg1)
{
    PID_par_cvl = arg1;
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
}
void controller::PID_p_tau_d(int arg1)
{
    PID_par_tau_d = arg1;
}

void controller::PID_update_samp(int arg1)
{
    PID_update_out_sec = arg1;
}

