#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QSettings>
#include <QTime>
#include <wiringPi.h>
#include "save_dialog.h"
#include "shunt3_controller.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void setheatpump(QVector<int>);
    void test(void);
    void PID_feedback(double);
    void PID_setpoint(double);
    void PID_p_cvu(int);
    void PID_p_cvl(int);
    void PID_p_p(double);
    void PID_p_i(double);
    void PID_p_d(double);
    void PID_p_tau_i(int);
    void PID_p_tau_d(int);
    void PID_update_samp(int);
    void PID_forward(double);
    void controller_mode(int);
    void shunt2_contr_ON(int);
    void shunt2_cvu_par(double);
    void shunt2_cvl_par(double);
    void shunt2_gain_par(double);
    void shunt2_i_par(double);
    void shunt2_d_par(double);
    void shunt2_tau_i(double);
    void shunt2_tau_d(double);
    void shunt2_hyst_par(double);
    void radiator_temp2(double);

    void shunt3_temperature_fb(double);
    void shunt3_contr_ON(int);

public slots:
    void temperatures(QVector<float>);
    void temp_id(QVector<QString>);
    void PID_control_signal(double);//Downsampled control signal
    void PID_control_instant_signal(double);//Not down sample control signal
    void indicator_shunt2_cw(bool);
    void indicator_shunt2_ccw(bool);
    void indicator_shunt2_d_part(double);
    void indicator_shunt2_d_filt(double);

private slots:

    void on_checkBox_on_off_clicked(bool checked);

    void on_spinBox_man_temp_hp_valueChanged(int arg1);


    void on_checkBox_hotwater_mode_clicked(bool checked);

    void on_checkBox_heater_mode_clicked(bool checked);

    void on_spinBox_manual_hotwater_valueChanged(int arg1);

    void on_checkBox_hotwater_and_heater_mode_clicked(bool checked);

    void on_checkBox_manual_clicked(bool checked);

    void on_checkBox_auto_clicked(bool checked);

    void heatpumpreply(QVector<int>);
    void controllertick(void);

    void on_checkBox_heater_mode_toggled(bool checked);

    void on_checkBox_hotwater_mode_toggled(bool checked);

    void on_checkBox_hotwater_and_heater_mode_toggled(bool checked);

    void on_checkBox_on_off_toggled(bool checked);


    void on_spinBox_temp_to_inhouse_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_2_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_3_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_4_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_5_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_6_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_7_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_8_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_9_valueChanged(int arg1);

    void on_spinBox_temp_to_inhouse_10_valueChanged(int arg1);

    void on_doubleSpinBox_pid_p_valueChanged(double arg1);

    void on_spinBox_mixer_valueChanged(int arg1);


    void on_doubleSpinBox_offset_forward_valueChanged(double arg1);



    void on_spinBox_pid_cvl_valueChanged(int arg1);

    void on_spinBox_pid_cvu_valueChanged(int arg1);

    void on_doubleSpinBox_pid_i_valueChanged(double arg1);

    void on_doubleSpinBox_pid_d_valueChanged(double arg1);

    void on_spinBox_pid_reset_tau_valueChanged(int arg1);

    void on_spinBox_pid_d_tau_valueChanged(int arg1);

    void on_spinBox_pid_control_samp_valueChanged(int arg1);

    void on_doubleSpinBox_gain_forward_valueChanged(double arg1);


    void on_verticalSlider_0_valueChanged(int value);

    void on_verticalSlider_1_valueChanged(int value);

    void on_verticalSlider_2_valueChanged(int value);

    void on_verticalSlider_3_valueChanged(int value);

    void on_verticalSlider_4_valueChanged(int value);

    void on_verticalSlider_5_valueChanged(int value);

    void on_verticalSlider_6_valueChanged(int value);

    void on_verticalSlider_7_valueChanged(int value);

    void on_verticalSlider_8_valueChanged(int value);

    void on_verticalSlider_9_valueChanged(int value);

    void on_verticalSlider_10_valueChanged(int value);

    void on_verticalSlider_11_valueChanged(int value);

    void on_verticalSlider_12_valueChanged(int value);

    void on_verticalSlider_13_valueChanged(int value);

    void on_verticalSlider_14_valueChanged(int value);

    void on_verticalSlider_15_valueChanged(int value);

    void on_verticalSlider_16_valueChanged(int value);

    void on_verticalSlider_17_valueChanged(int value);

    void on_verticalSlider_18_valueChanged(int value);

    void on_verticalSlider_19_valueChanged(int value);

    void on_verticalSlider_20_valueChanged(int value);

    void on_verticalSlider_21_valueChanged(int value);

    void on_verticalSlider_22_valueChanged(int value);

    void on_verticalSlider_23_valueChanged(int value);

    void on_spinBox_auto_off_actual_valueChanged(int arg1);


    void on_checkBox_shunt2_fire_clicked(bool checked);

    void on_checkBox_shunt2_man_pool_clicked(bool checked);

    void on_checkBox_shunt2_auto_pool_clicked(bool checked);

    void on_doubleSpinBox_shunt2_gain_valueChanged(double arg1);

    void on_doubleSpinBox_shunt2_hyst_valueChanged(double arg1);


    void on_pushButton_clicked();
    void user_save_settings(bool);

    void on_doubleSpinBox_pid_shunt2_cvu_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt2_cvl_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt2_i_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt2_d_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt2_tau_i_valueChanged(double arg1);

    void on_doubleSpinBox_pid_shunt2_tau_d_valueChanged(double arg1);

    void on_spinBox_shunt2_auto_off_valueChanged(int arg1);

    void on_pushButton_2_clicked();

    void on_checkBox_shunt3_heat_acc_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    save_dialog *save_obj;
    shunt3_controller *shunt3obj1;

    QVector<int> heatpump_send;
    QVector<int> heatpump_reply;
    QVector<int> temp_connection_matrix;
    QVector<double> temperature_inp;
    QVector<double> temperature_matrix;
    QPixmap *radiator_pix;
    QPixmap *tap_water_pix;
    QPixmap *OFF_pix;
    QPixmap *Plus_pix;
    QPixmap *Plus_pix_b;
    QPixmap *Minus_pix;
    QPixmap *error_pix;
    int start_up;
    bool auto_init_done;
    int tick_cnt1;
    void set_radiator_mode(void);
    void set_radiator_temp(int);
    void set_tap_water_mode(void);
    void set_tap_water_temp(int);
    void set_both_mode(void);
    void gray_out_user(void);
    void auto_mode_turn_on(void);
    void man_mode_checkbox_update(void);
    void emit_PID_parameters(void);
    void update_profile_lable(void);
    void solar_to_pool(void);
    void solar_to_hotwater(void);
    void set_shunt2to_cw(void);
    void save_settings(void);
    void save_dialog_quest(void);

    //Project File
    //mySettings = new QSettings("ProjectName", "applicationName");
    QString WorkSettingsPath;//ProjectName path
    QString WorkSettingsFile;//Project file name
    QSettings *mySettings;

    int Mixer_inhouse_1;
    int Mixer_inhouse_2;
    double inhouse_temp;
    double forward_temp;
    double forward_signal;
    double temp_setp_1;
    double temp_setp_with_profile;
    double temp_profile;
    double outside_temp;
    double hysteres_auto_off;
    double hot_w_temp_sensor;
    int debug_reinit_low_temp_hot_w;
    int reinit_timer;
    double hot_w_low_threshold_b;
    double high_temp_hot_w_th_b;
    bool switch_mode;
    QTime time;
    int alive_GPIO;

    double solar_exchanger;
    double pool_exchanger;
    double solar_top_diff_ON;
    double solar_top_diff_OFF;
    bool solar2pool_state;
    double pump3_hyst;
    int fire_auto_off_timer;
    double hys_overh_1;
    double hys_overh_2;


};

#endif // MAINWINDOW_H
