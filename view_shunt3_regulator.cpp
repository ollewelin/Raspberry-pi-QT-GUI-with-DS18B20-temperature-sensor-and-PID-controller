#include "view_shunt3_regulator.h"
#include "ui_view_shunt3_regulator.h"

view_shunt3_regulator::view_shunt3_regulator(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::view_shunt3_regulator)
{
    ui->setupUi(this);
}

view_shunt3_regulator::~view_shunt3_regulator()
{
    delete ui;
}
void view_shunt3_regulator::init_with_mySett(void)
{

    ui->doubleSpinBox_shunt3_gain->setValue(mySettings->value("mySettings/shunt3_gain", "").toDouble());
    ui->doubleSpinBox_shunt3_hyst->setValue(mySettings->value("mySettings/shunt3_hyst", "").toDouble());
    ui->doubleSpinBox_pid_shunt3_cvu->setValue(mySettings->value("mySettings/shunt3_cvu_par", "").toDouble());
    ui->doubleSpinBox_pid_shunt3_cvl->setValue(mySettings->value("mySettings/shunt3_cvl_par", "").toDouble());
    ui->doubleSpinBox_pid_shunt3_i->setValue(mySettings->value("mySettings/shunt3_i_par", "").toDouble());
    ui->doubleSpinBox_pid_shunt3_d->setValue(mySettings->value("mySettings/shunt3_d_par", "").toDouble());
    ui->doubleSpinBox_pid_shunt3_tau_i->setValue(mySettings->value("mySettings/shunt3_tau_i", "").toDouble());
    ui->doubleSpinBox_pid_shunt3_tau_d->setValue(mySettings->value("mySettings/shunt3_tau_d", "").toDouble());
    ui->doubleSpinBox_shunt3_setp->setValue(mySettings->value("mySettings/shunt3_temperatur_setpoint", "").toDouble());

    emit shunt3_gain_par(ui->doubleSpinBox_shunt3_gain->value());
    emit shunt3_hyst_par(ui->doubleSpinBox_shunt3_hyst->value());
    emit shunt3_cvu_par(ui->doubleSpinBox_pid_shunt3_cvu->value());
    emit shunt3_cvl_par(ui->doubleSpinBox_pid_shunt3_cvl->value());
    emit shunt3_i_par(ui->doubleSpinBox_pid_shunt3_i->value());
    emit shunt3_d_par(ui->doubleSpinBox_pid_shunt3_d->value());
    emit shunt3_tau_d(ui->doubleSpinBox_pid_shunt3_tau_d->value());
    emit shunt3_tau_i(ui->doubleSpinBox_pid_shunt3_tau_i->value());
    emit shunt3_temperatur_setpoint(ui->doubleSpinBox_shunt3_setp->value());
}

void view_shunt3_regulator::on_doubleSpinBox_shunt3_gain_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_gain"), y);
    emit shunt3_gain_par(arg1);
}

void view_shunt3_regulator::on_doubleSpinBox_pid_shunt3_cvu_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_cvu_par"), y);
    emit shunt3_cvu_par(arg1);
}

void view_shunt3_regulator::on_doubleSpinBox_pid_shunt3_cvl_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_cvl_par"), y);
    emit shunt3_cvl_par(arg1);
}

void view_shunt3_regulator::on_doubleSpinBox_pid_shunt3_i_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_i_par"), y);
    emit shunt3_i_par(arg1);
}

void view_shunt3_regulator::on_doubleSpinBox_pid_shunt3_d_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_d_par"), y);
    emit shunt3_d_par(arg1);
}

void view_shunt3_regulator::on_doubleSpinBox_pid_shunt3_tau_i_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_tau_i"), y);
    emit shunt3_tau_i(arg1);
}

void view_shunt3_regulator::on_doubleSpinBox_pid_shunt3_tau_d_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_tau_d"), y);
    emit shunt3_tau_d(arg1);
}

void view_shunt3_regulator::on_doubleSpinBox_shunt3_hyst_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_hyst"), y);
    emit shunt3_hyst_par(arg1);
}

void view_shunt3_regulator::on_doubleSpinBox_shunt3_setp_valueChanged(double arg1)
{
    QString y = QString::number(arg1, 10, 9);
    mySettings->setValue(QString("mySettings/shunt3_temperatur_setpoint"), y);
    emit shunt3_temperatur_setpoint(arg1);

}


void view_shunt3_regulator::indicator_shunt3_cw(bool arg1)
{
    ui->checkBox_shunt3_cw->setChecked(arg1);
}
void view_shunt3_regulator::indicator_shunt3_ccw(bool arg1)
{
    ui->checkBox_shunt3_ccw->setChecked(arg1);
}
void view_shunt3_regulator::shunt3_D_filt(double arg1)
{
    ui->lineEdit_shunt3_D_filt->setText(QString::number(arg1, 'f', 7));
}
void view_shunt3_regulator::shunt3_D_part(double arg1)
{
    ui->lineEdit_shunt3_D_part->setText(QString::number(arg1, 'f', 7));
}
void view_shunt3_regulator::shunt3_I_part(double arg1)
{
    ui->lineEdit_shunt3_I_part->setText(QString::number(arg1, 'f', 7));
}
void view_shunt3_regulator::shunt3_P_part(double arg1)
{
    ui->lineEdit_shunt3_P_part->setText(QString::number(arg1, 'f', 7));
}
void view_shunt3_regulator::shunt3_control_value(double arg1)
{
    ui->lineEdit_shunt3_cv->setText(QString::number(arg1, 'f', 7));
}
