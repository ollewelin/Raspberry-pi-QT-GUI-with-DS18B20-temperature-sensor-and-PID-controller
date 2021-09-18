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


private:
    Ui::MainWindow *ui;
    QVector<int> heatpump_send;
    QVector<int> heatpump_reply;
    QPixmap *radiator_pix;
    QPixmap *tap_water_pix;
    QPixmap *OFF_pix;
    QPixmap *Plus_pix;
    QPixmap *Plus_pix_b;
    QPixmap *Minus_pix;
    int start_up;
    int tick_cnt1;
    void set_radiator_mode(void);
    void set_tap_water_mode(void);
    void set_both_mode(void);
    //Project File
    //mySettings = new QSettings("ProjectName", "applicationName");
    QString WorkSettingsPath;//ProjectName path
    QString WorkSettingsFile;//Project file name
    QSettings *mySettings;

};

#endif // MAINWINDOW_H
