#ifndef TESTQCLASS_H
#define TESTQCLASS_H

#include <QObject>

class testqclass : public QObject
{
    Q_OBJECT
public:
    explicit testqclass(QObject *parent = nullptr);

signals:

public slots:
};

#endif // TESTQCLASS_H
