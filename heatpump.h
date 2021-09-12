#ifndef HEATPUMP_H
#define HEATPUMP_H

#include <QObject>
#include"heatpump_socket_client.h"

class heatpump : public QObject
{
    Q_OBJECT
public:
    explicit heatpump(QObject *parent = nullptr);
    int nrofheatpsignals;
signals:
    void replyheatpump(QVector<int>);

public slots:
    void setheatpump(QVector<int>);
    void test(void);
private:
    QVector<int> main_qt_thread_heatpump_send;//local copy of temperature from heatpump thread, copyed during this main qt thread have lock the mutex.
    QVector<int> main_qt_thread_heatpump_recive;//
    int main_qt_thread_heatpump_are_initialized;
    heatpump_socket_client *heatpThread_a;
    pthread_mutex_t *mut2;

};

#endif // HEATPUMP_H
