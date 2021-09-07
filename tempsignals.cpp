#include "tempsignals.h"


//tempsignals class contain the QT thread side of the temperature signals.
//This class will start the separate tempsens thread reading the DS18B20 sensors and copy over that data to this QT tread

tempsignals::tempsignals(QObject *parent) : QObject(parent)
{
    main_qt_thread_temps_are_initialized = 0;
    main_qt_thread_temperature.clear();
    //****** Create a Temperature sensor thread ***************
    mut = new pthread_mutex_t;
    pthread_mutex_init(mut, NULL);
    tempThread_a = new tempsens(mut);

    // Start the thread, send it the this pointer (points to this class
    // instance)
    tempThread_a->Start();

    //*********************************************************

}
void tempsignals::gettemperature(void)
{

    int pre_are_init = main_qt_thread_temps_are_initialized;
    pthread_mutex_lock(mut);
    main_qt_thread_temps_are_initialized = tempThread_a->temps_are_initialized;
    pthread_mutex_unlock(mut);
    if(pre_are_init != main_qt_thread_temps_are_initialized){
        emit TempsensorInit(main_qt_thread_temps_are_initialized);
        if(main_qt_thread_temps_are_initialized == 1){
            //tempsensor OK
            emit Rom_vect(tempThread_a->rom_vect);
            for(int i=0;i<tempThread_a->rom_vect.size();i++){
                main_qt_thread_temperature.push_back(0.0f);
            }
        }
    }

    if(main_qt_thread_temps_are_initialized == 1){
        pthread_mutex_lock(mut);
        main_qt_thread_temperature = tempThread_a->temperature;
        pthread_mutex_unlock(mut);
        emit Temperature(main_qt_thread_temperature);
        printf("Emit temperature vector\n");
    }
}
