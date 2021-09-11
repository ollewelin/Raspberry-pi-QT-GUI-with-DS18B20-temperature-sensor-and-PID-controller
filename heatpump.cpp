#include "heatpump.h"
#include"heatpump_socket_client.h"

heatpump::heatpump(QObject *parent) : QObject(parent)
{
    main_qt_thread_heatpump_are_initialized = 0;
    main_qt_thread_heatpump_send.clear();
    main_qt_thread_heatpump_recive.clear();
    //****** Create a Temperature sensor thread ***************
    mut2 = new pthread_mutex_t;
    pthread_mutex_init(mut2, NULL);
    heatpThread_a = new heatpump_socket_client(mut2);
    // Start the thread, send it the this pointer (points to this class
    // instance)
    heatpThread_a->Start();
    //*********************************************************

}

