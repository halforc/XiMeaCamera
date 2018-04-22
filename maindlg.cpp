#include "maindlg.h"
#include "ui_maindlg.h"
#include <QDebug>
#include <QPainter>
#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <qxmlstream.h>

#include <QSlider>
#include <QFileDialog>


mainDlg::mainDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainDlg)
{
    //设置界面主题
    QFile file("..//XiMeaCamera//style//newstyle.qss");
    if(!file.open(QFile::ReadOnly)){
        qDebug()<<"open failed";
    }
    QString styleSheet = QLatin1String(file.readAll());
    //qApp->setStyleSheet(styleSheet);
    file.close();

    //固定窗口大小
    ui->setupUi(this);
    this->setFixedSize(this->size());

    //COMMbox
    ui->comDPI->addItem("320*240");
    ui->comDPI->addItem("1024*768");
    QLineEdit* l = new QLineEdit();
    l->setText(ui->comDPI->currentText());
    l->setAlignment(Qt::AlignHCenter);
    ui->comDPI->setLineEdit(l);

    //事件关联
    ui->leExposureTime->setText(QString::number(ui->sliderExposureTime->value()));
    ui->leFocalDistance->setText(QString::number(ui->sliderFocalDistance->value()));
    ui->leFrameRate->setText(QString::number(ui->sliderFrameRate->value()));
    ui->leLensAperture->setText(QString::number(ui->sliderLensApersure->value()));
    ui->sliderExposureTime->setMinimum(MIN_EXPOSURE_TIME);
    ui->sliderExposureTime->setMaximum(MAX_EXPOSURE_TIME);
    ui->sliderExposureTime->setSingleStep(INC_EXPOSURE_TIME);
    ui->sliderExposureTime->installEventFilter(this);
    ui->sliderFocalDistance->installEventFilter(this);
    ui->sliderFrameRate->installEventFilter(this);
    ui->sliderLensApersure->installEventFilter(this);
}

mainDlg::~mainDlg()
{
    delete ui;
}

void mainDlg::paintEvent(QPaintEvent *event)
{

}


void mainDlg::on_test_clicked()
{
//    xiAPIplusCameraOcv cam;
//    cam.OpenFirst();
//    cam.SetExposureTime(10000);
//    cam.StartAcquisition();
//    XI_IMG_FORMAT format = cam.GetImageDataFormat();
//    cv_mat_image = cam.GetNextImageOcvMat();
//    if (format == XI_RAW16 || format == XI_MONO16)
//        cv::normalize(cv_mat_image, cv_mat_image, 0, 65536, cv::NORM_MINMAX, -1, cv::Mat()); // 0 - 65536, 16 bit unsigned integer range

//    QPixmap pixmap = QPixmap::fromImage(Mat2QImage(cv_mat_image));
//    int with = ui->label->width();
//    int height = ui->label->height();
//    pixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
//    ui->label->setPixmap(pixmap);

//    qDebug()<<"width: "<<cam.GetWidth();
//    qDebug()<<"height: "<<cam.GetHeight();
//    cam.StopAcquisition();
//    cam.Close();
    //qDebug()<<"test";
}

void mainDlg::on_open_clicked()
{
    qDebug()<<"on_open_clicked";
}

void mainDlg::on_record_clicked()
{
    qDebug()<<"on_record_clicked";
}

void mainDlg::on_replay_clicked()
{
    qDebug()<<"on_replay_clicked";
//    QString str = QFileDialog::getOpenFileName(this,
//                     "Select one or more files to open",
//                     "/home",
//                     "Images (*.png *.xpm *.jpg)");
//    QPixmap pixmap(str);
//    int with = ui->label->width();
//    int height = ui->label->height();
//    pixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
//    ui->label->setPixmap(pixmap);
    t = new playercontrol(this);
    t->move(0,430);
    t->show();
    connect(this,SIGNAL(test(QLabel*)),t,SLOT(openPlayer(QLabel*)));
    emit test(ui->label);
}

void mainDlg::on_close_clicked()
{
    qDebug()<<"on_close_clicked";
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

void mainDlg::initial()
{

}

void mainDlg::readDevParaFromXML(DEVICE_SETTING *pDevInfo)
{
   QFileInfo fileInfo("XiMeaCamearaConfig.xml");
   QFile file("XiMeaCamearaConfig.xml");
   if(!fileInfo.exists()){

   }
   if(file.open(QIODevice::WriteOnly | QIODevice::Text))
   {
       QXmlStreamReader xmlReater(&file);
       while(xmlReater.atEnd()){
           if(xmlReater.name() == tr("exposureTime")){
               pDevInfo->exposureTime = xmlReater.readElementText().toDouble();
           }
           if(xmlReater.name() == tr("lensAperture")){
               pDevInfo->lensAperture = xmlReater.readElementText().toDouble();
           }
           if(xmlReater.name() == tr("frameRate")){
               pDevInfo->frameRate = xmlReater.readElementText().toDouble();
           }
           if(xmlReater.name() == tr("focusDistance")){
               pDevInfo->focusDistance = xmlReater.readElementText().toDouble();
           }
           if(xmlReater.name() == tr("width")){
               pDevInfo->width = xmlReater.readElementText().toDouble();
           }
           if(xmlReater.name() == tr("height")){
               pDevInfo->height = xmlReater.readElementText().toDouble();
           }
           xmlReater.readNext();
       }
       file.close();
   }   
}

void mainDlg::writeDevParaToXML(DEVICE_INFO *pDevInfo)
{
    QFile file("XiMeaCamearaConfig.xml");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }
    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    //XiMea Cameara
    xmlWriter.writeStartElement("XiMea Cameara USB3.0");
    //相机参数
    xmlWriter.writeStartElement("camera parameter");

    //曝光时间（微秒）
    xmlWriter.writeStartElement("exposureTime");
    xmlWriter.writeTextElement("curExposureTime",QString::number(m_xiCam.GetExposureTime()));
    xmlWriter.writeTextElement("miniExposureTime",QString::number(m_xiCam.GetExposureTime_Minimum()));
    xmlWriter.writeTextElement("maxExposureTime",QString::number(m_xiCam.GetExposureTime_Maximum()));
    xmlWriter.writeTextElement("exposureTimeIncrement",QString::number(m_xiCam.GetExposureTime_Increment()));
    xmlWriter.writeEndElement();

    //光圈(float)
    xmlWriter.writeStartElement("lensAperture");
    xmlWriter.writeTextElement("curLensAperture",QString::number(m_xiCam.GetLensApertureValue()));
    xmlWriter.writeTextElement("miniLensAperture",QString::number(m_xiCam.GetLensApertureValue_Minimum()));
    xmlWriter.writeTextElement("maxLensAperture",QString::number(m_xiCam.GetLensApertureValue_Maximum()));
    xmlWriter.writeTextElement("lensApertureIncrement",QString::number(m_xiCam.GetLensApertureValue_Increment()));
    xmlWriter.writeEndElement();

    //帧率(float)
    xmlWriter.writeStartElement("frameRate");
    xmlWriter.writeTextElement("curFrameRate",QString::number(m_xiCam.GetFrameRate()));
    xmlWriter.writeTextElement("miniFrameRate",QString::number(m_xiCam.GetFrameRate_Minimum()));
    xmlWriter.writeTextElement("maxFrameRate",QString::number(m_xiCam.GetFrameRate_Maximum()));
    xmlWriter.writeTextElement("frameRateIncrement",QString::number(m_xiCam.GetFrameRate_Increment()));
    xmlWriter.writeEndElement();

    //焦距（mm）
    xmlWriter.writeStartElement("focusDistance");
    xmlWriter.writeTextElement("curFocusDistance",QString::number(m_xiCam.GetLensFocalLength()));
    xmlWriter.writeTextElement("miniFocusDistance",QString::number(m_xiCam.GetLensFocalLength_Minimum()));
    xmlWriter.writeTextElement("maxFocusDistance",QString::number(m_xiCam.GetLensFocalLength_Maximum()));
    xmlWriter.writeTextElement("focusDistanceIncrement",QString::number(m_xiCam.GetLensFocalLength_Increment()));
    xmlWriter.writeEndElement();


    //图像宽度（像素int）
    xmlWriter.writeStartElement("widthInfo");
    xmlWriter.writeTextElement("curWidth",QString::number(m_xiCam.GetWidth()));
    xmlWriter.writeTextElement("miniWidth",QString::number(m_xiCam.GetWidth_Minimum()));
    xmlWriter.writeTextElement("maxWidth",QString::number(m_xiCam.GetWidth_Maximum()));
    xmlWriter.writeTextElement("widthIncrement",QString::number(m_xiCam.GetWidth_Increment()));
    xmlWriter.writeEndElement();

    //图像高度（像素int）
    xmlWriter.writeStartElement("sizeInfo");
    xmlWriter.writeTextElement("curHeight",QString::number(m_xiCam.GetHeight()));
    xmlWriter.writeTextElement("miniHeight",QString::number(m_xiCam.GetHeight_Minimum()));
    xmlWriter.writeTextElement("maxHeight",QString::number(m_xiCam.GetHeight_Maximum()));
    xmlWriter.writeTextElement("heightIncrement",QString::number(m_xiCam.GetHeight_Increment()));
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();        //XiMea Cameara
    xmlWriter.writeEndDocument();        //相机参数
    xmlWriter.writeEndDocument();

    file.close();
}

bool mainDlg::eventFilter(QObject * obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease){
        if(obj == ui->sliderExposureTime)
            ui->leExposureTime->setText(QString::number(ui->sliderExposureTime->value()));
        if(obj == ui->sliderFocalDistance)
            ui->leFocalDistance->setText(QString::number(ui->sliderFocalDistance->value()));
        if(obj == ui->sliderFrameRate)
            ui->leFrameRate->setText(QString::number(ui->sliderFrameRate->value()));
        if(obj == ui->sliderLensApersure)
            ui->leLensAperture->setText(QString::number(ui->sliderLensApersure->value()));
    }

    return QObject::eventFilter(obj, event);
}

