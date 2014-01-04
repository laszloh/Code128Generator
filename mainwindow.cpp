#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QMessageBox>

#include "gencode128.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label_2->setStyleSheet("QLabel {color: black}");
    ui->widget->setStyleSheet("QWidget {background-color: white;}");
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
        ui->widget->render(&printer);
    }
}

void MainWindow::on_lineEdit_textChanged(const QString &str)
{
    ui->label->setText(str);
}
