#ifndef GROUPWINDOW_H
#define GROUPWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>

class GroupWindow : public QMainWindow,private Ui::MainWindow
{
    Q_OBJECT
public:
    explicit GroupWindow(QWidget *parent = 0);

signals:

public slots:

};

#endif // GROUPWINDOW_H
