#include "heatpump.h"
#include"heatpump_socket_client.h"


//Communication between server and this client
//regarding heatpump_server.py socket server.
//1. start a socket server for communicate with a socket (from a GUI QT application with a heatpump_socket_clinet.cpp class)
//2. Inside this socket server while True: loop command and data reply will be handle with use of the tinytuya API
//(a tuya API how use the LocalKey not using the Cloud)

//The command and reply data is in a form of 10 array c_uint32 data send/recive
//array index [0] = "command"
//    0=Turn OFF heatpump device
//    1=Turn ON heatpump device
//    2=hot_hotwater mode (heater + hotwater mode selected)
//    3=hotwater mode (Only hotwater mode selected)
//    4=hot mode (Only heater mode)
//    5=set_temp
//    6=only_read_status

//    10=cool mode, not implemented yet
//    11=cool_hotwater mode, not implemented yet
//array index [1] = setpoint temperature
//array index [2] = readback ON/OFF
//array index [3] = readback actual temperature
//array index [4] = readback setpoint temperature
//array index [5] = readback mode, mode readback code same as array index [0] = "command"

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

void heatpump::setheatpump(QVector<int> heatpump_send_vect)
{
    if(heatpThread_a->socket_send.size() == heatpump_send_vect.size() && heatpThread_a->socket_receive.size() == heatpump_send_vect.size())
    {
        pthread_mutex_lock(mut2);
        main_qt_thread_heatpump_are_initialized = heatpThread_a->socket_initialized;
        pthread_mutex_unlock(mut2);
        if(main_qt_thread_heatpump_are_initialized == 1){
            main_qt_thread_heatpump_send = heatpump_send_vect;
            pthread_mutex_lock(mut2);
            heatpThread_a->socket_send = main_qt_thread_heatpump_send;
            pthread_mutex_unlock(mut2);
            pthread_mutex_lock(mut2);
            main_qt_thread_heatpump_recive = heatpThread_a->socket_receive;
            pthread_mutex_unlock(mut2);
            emit replyheatpump(main_qt_thread_heatpump_recive);
        }
    }
    else {
        //error
        printf("ERROR! internal program error, heatpThread_a->socket_send.size() != heatpump_send_vect.size()\n");
        printf("Exit program\n");
        exit(0);
    }
}
