#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
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

private:
    Ui::MainWindow *ui;
    QVector<int> heatpump_send;
    QVector<int> heatpump_reply;
    int last_selected_hotwater_temp;
    int last_selected_hot_temp;

};

#endif // MAINWINDOW_H
