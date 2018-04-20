#include "maindlg.h"
#include "ui_maindlg.h"
#include <QDebug>
#include <QPainter>
#include <QImage>
#include <QFile>

mainDlg::mainDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainDlg)
{
    QFile file("D:\\Qt\\project\\XiCameraCtrl\\style\\newstyle.qss");
    if(!file.open(QFile::ReadOnly)){
        qDebug()<<"open failed";
    }
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
    file.close();

    ui->setupUi(this);

    mTimer = new QTimer(this);
    connect(mTimer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
//    img.load("D://test.jpg");
//    ui->label->setPixmap(QPixmap::fromImage(img));

    ui->comDPI->addItem("320*240");
    ui->comDPI->addItem("1024*768");
    QLineEdit* l = new QLineEdit();
    l->setText(ui->comDPI->currentText());
    l->setAlignment(Qt::AlignHCenter);
    ui->comDPI->setLineEdit(l);
}

mainDlg::~mainDlg()
{
    delete ui;
}

void mainDlg::paintEvent(QPaintEvent *event)
{

}

void mainDlg::onTimeOut()
{

}

void mainDlg::on_test_clicked()
{
    xiAPIplusCameraOcv cam;
    cam.OpenFirst();
    cam.SetExposureTime(10000);
    cam.StartAcquisition();
    XI_IMG_FORMAT format = cam.GetImageDataFormat();
    cv_mat_image = cam.GetNextImageOcvMat();
    if (format == XI_RAW16 || format == XI_MONO16)
        cv::normalize(cv_mat_image, cv_mat_image, 0, 65536, cv::NORM_MINMAX, -1, cv::Mat()); // 0 - 65536, 16 bit unsigned integer range

    QPixmap pixmap = QPixmap::fromImage(Mat2QImage(cv_mat_image));
    int with = ui->label->width();
    int height = ui->label->height();
    pixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
    ui->label->setPixmap(pixmap);

    qDebug()<<"width: "<<cam.GetWidth();
    qDebug()<<"height: "<<cam.GetHeight();
    cam.StopAcquisition();
    cam.Close();

}

QImage mainDlg::Mat2QImage(cv::Mat cvImg)
{
    QImage qImg;
    if(cvImg.channels() == 3){
        cvtColor(cvImg,cvImg,CV_BGR2RGB);
        qImg = QImage((const uchar*)(cvImg.data),
                      cvImg.cols,cvImg.rows,
                      cvImg.cols * cvImg.channels(),
                      QImage::Format_RGB888);
    }
    else if(cvImg.channels() == 1){
        qImg = QImage((const uchar*)(cvImg.data),
                      cvImg.cols,cvImg.rows,
                      cvImg.cols * cvImg.channels(),
                      QImage::Format_Indexed8);
    }
    else{
        qImg = QImage((const uchar*)(cvImg.data),
                      cvImg.cols,cvImg.rows,
                      cvImg.cols * cvImg.channels(),
                      QImage::Format_RGB888);
    }
    return qImg;
}

