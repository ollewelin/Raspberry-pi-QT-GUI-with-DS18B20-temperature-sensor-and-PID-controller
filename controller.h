#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
class controller : public QObject
{
    Q_OBJECT
public:
    explicit controller(QObject *parent = nullptr);

signals:
    void controllertick(void);
public slots:
    void timetick(void);
private:
    QTimer *sample_timer;//timer driver for controller sampling rate
    int timetickcnt;
};

#endif // CONTROLLER_H
