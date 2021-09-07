#ifndef TEMPSIGNALS_H
#define TEMPSIGNALS_H

//tempsignals class contain the QT thread side of the temperature signals.
//This class will start the separate tempsens thread reading the DS18B20 sensors and copy over that data to this QT tread

#include <QObject>
#include <QVector>
#include "tempsens.h"
class tempsignals : public QObject
{
    Q_OBJECT
public:
    explicit tempsignals(QObject *parent = nullptr);

signals:
    void Temperature(QVector<float>);
    void Rom_vect(QVector<QString>);
    void TempsensorInit(int);

public slots:
    void gettemperature(void);

private:
    QVector<float> main_qt_thread_temperature;//local copy of temperature from tempsensor thread, copyed during this main qt thread have lock the mutex.
    int main_qt_thread_temps_are_initialized;
    tempsens *tempThread_a;
    pthread_mutex_t *mut;

};

#endif // TEMPSIGNALS_H
