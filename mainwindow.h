#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QSettings>

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


public slots:
    void temperatures(QVector<float>);
    void temp_id(QVector<QString>);


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

    void on_doubleSpinBox_gain_forward_valueChanged(double arg1);

    void on_doubleSpinBox_offset_forward_valueChanged(double arg1);

    void on_doubleSpinBox_inhouse_setp_valueChanged(double arg1);



private:
    Ui::MainWindow *ui;
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


    //Project File
    //mySettings = new QSettings("ProjectName", "applicationName");
    QString WorkSettingsPath;//ProjectName path
    QString WorkSettingsFile;//Project file name
    QSettings *mySettings;

    double PID_par_cvu;
    double PID_par_cvl;
    double PID_par_p;
    double PID_par_i;
    double PID_par_d;
    double PID_par_tau_i;
    double PID_par_tau_d;
    double PID_forward_gain;
    double PID_forward_offset;

    int Mixer_inhouse_1;
    int Mixer_inhouse_2;
    double inhouse_temp;
    double forward_temp;
    double forward_signal;
    double temp_setp_1;
    double temp_setp_with_profile;
    double temp_profile;
    double outside_temp;

    bool switch_mode;




};

#endif // MAINWINDOW_H
