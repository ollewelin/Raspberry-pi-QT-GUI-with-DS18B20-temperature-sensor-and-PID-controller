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
