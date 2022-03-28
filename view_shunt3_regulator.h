#ifndef VIEW_SHUNT3_REGULATOR_H
#define VIEW_SHUNT3_REGULATOR_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class view_shunt3_regulator;
}

class view_shunt3_regulator : public QDialog
{
    Q_OBJECT

public:
    explicit view_shunt3_regulator(QWidget *parent = nullptr);
    ~view_shunt3_regulator();

    QSettings *mySettings;
    void init_with_mySett(void);

signals:
    void shunt3_temperatur_setpoint(double);
    void shunt3_cvu_par(double);
    void shunt3_cvl_par(double);
    void shunt3_gain_par(double);
    void shunt3_i_par(double);
    void shunt3_d_par(double);
    void shunt3_tau_i(double);
    void shunt3_tau_d(double);
    void shunt3_hyst_par(double);

public slots:
    void indicator_shunt3_cw(bool);
    void indicator_shunt3_ccw(bool);
    void shunt3_D_filt(double);
    void shunt3_D_part(double);
    void shunt3_I_part(double);
    void shunt3_P_part(double);
    void shunt3_control_value(double);

private slots:

    void on_doubleSpinBox_shunt3_gain_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt3_cvu_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt3_cvl_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt3_i_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt3_d_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt3_tau_i_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt3_tau_d_valueChanged(double arg1);

    void on_doubleSpinBox_shunt3_hyst_valueChanged(double arg1);

    void on_doubleSpinBox_shunt3_setp_valueChanged(double arg1);

private:
    Ui::view_shunt3_regulator *ui;
};

#endif // VIEW_SHUNT3_REGULATOR_H
