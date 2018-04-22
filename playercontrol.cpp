#include "playercontrol.h"
#include "ui_playercontrol.h"

#include <QFileDialog>
#include <QPixmap>
#include <QDebug>
playercontrol::playercontrol(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::playercontrol)
{
    ui->setupUi(this);

//    setAttribute(Qt::WA_TranslucentBackground, true);
//    ui->pushButton->setFlat(true);
}

playercontrol::~playercontrol()
{
    delete ui;
}

void playercontrol::openPlayer(QLabel *label)
{
    qDebug()<<"open...";
    QString str = QFileDialog::getOpenFileName(this,
                     "Select one or more files to open",
                     "/home",
                     "Images (*.png *.xpm *.jpg)");
    QPixmap pixmap(str);
    int with = label->width();
    int height = label->height();
    pixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
    label->setPixmap(pixmap);
}
