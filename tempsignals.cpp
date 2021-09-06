#include "tempsignals.h"
#include "tempsens.h"
tempsignals::tempsignals(QObject *parent) : QObject(parent)
{

    //****** Create a Temperature sensor thread ***************
    tempsens *tempThread_a;
    pthread_mutex_t *mut;
    mut = new pthread_mutex_t;
    pthread_mutex_init(mut, NULL);
    tempThread_a = new tempsens(mut);

    // Start the thread, send it the this pointer (points to this class
    // instance)
    tempThread_a->Start();

    //*********************************************************

}
