#ifndef TEMPSIGNALS_H
#define TEMPSIGNALS_H

#include <QObject>

class tempsignals : public QObject
{
    Q_OBJECT
public:
    explicit tempsignals(QObject *parent = nullptr);

signals:

public slots:
};

#endif // TEMPSIGNALS_H