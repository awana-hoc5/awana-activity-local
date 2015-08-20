#include "groupwindow.h"

GroupWindow::GroupWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    lbGName->setText("hello");
}
