#ifndef SAVE_DIALOG_H
#define SAVE_DIALOG_H

#include <QDialog>

namespace Ui {
class save_dialog;
}

class save_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit save_dialog(QWidget *parent = nullptr);
    ~save_dialog();

signals:
    void user_save_settings(bool);
private slots:
    void on_buttonBox_accepted();

private:
    Ui::save_dialog *ui;
};

#endif // SAVE_DIALOG_H
