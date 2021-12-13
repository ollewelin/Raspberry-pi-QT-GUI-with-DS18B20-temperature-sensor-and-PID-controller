#ifndef SHUNT2_CONTROLLER_H
#define SHUNT2_CONTROLLER_H

#include <QObject>
#include <QTimer>

class shunt2_controller : public QObject
{
    Q_OBJECT
public:
    explicit shunt2_controller(QObject *parent = nullptr);

signals:

public slots:
    void timetick(void);
    void PID_control_instant_signal(double);//No down sample
private:
    QTimer *sample_timer;//timer driver for controller sampling rate
    int timetickcnt;
    double sample_time;
    int shunt2_cnt;
    int blink;

};

#endif // SHUNT2_CONTROLLER_H
