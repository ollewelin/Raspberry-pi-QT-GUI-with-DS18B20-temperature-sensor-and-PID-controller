#include "shunt2_controller.h"
#include <wiringPi.h>

shunt2_controller::shunt2_controller(QObject *parent) : QObject(parent)
{
    sample_timer = new QTimer(this);
    sample_timer->start(100);//ms
    sample_time=0.1;//sec
    connect(sample_timer, SIGNAL(timeout()), this, SLOT(timetick()));//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
    printf("Constructor shunt2_controller\n");
    digitalWrite (1,  HIGH) ;
    blink=1;


}
void shunt2_controller::timetick(void)//This don't work for QCoreApplication::processEvents(QEventLoop::AllEvents);
{

    if(shunt2_cnt<100){
        shunt2_cnt++;
    }
    else {
        shunt2_cnt=0;
        printf("clear shunt2_cnt\n");
    }
    if(blink>0){
        blink=0;
        digitalWrite (1,  HIGH) ;
    }
    else{
        blink=1;
        digitalWrite (1,  LOW) ;
    }
}

void shunt2_controller::PID_control_instant_signal(double arg1)
{
    printf("PID_control_instant_signal = %f\n", (float)arg1);
}
