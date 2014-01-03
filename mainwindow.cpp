#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QPrinter printer;

    QPrintDialog printDialog(&printer, this);
    if(printDialog.exec() == QDialog::Accepted){
        this->render(&printer);
    }
}
