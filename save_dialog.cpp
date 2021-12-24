#include "save_dialog.h"
#include "ui_save_dialog.h"

save_dialog::save_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::save_dialog)
{
    ui->setupUi(this);
}

save_dialog::~save_dialog()
{
    delete ui;
}

void save_dialog::on_buttonBox_accepted()
{
    emit user_save_settings(true);
}
