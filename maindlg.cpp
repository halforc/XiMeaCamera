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
#include <QLineEdit>

#include <QCloseEvent>
#include <QMessageBox>

#include <QDateTime>
#include <QDir>
mainDlg::mainDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainDlg),
    m_acqThread(nullptr),
    m_objThread(nullptr),
    m_bIsRecording(false)
{
    //设置界面主题
    QFile file("..//XiMeaCameraCtrl//style//newstyle.qss");
    if(!file.open(QFile::ReadOnly)){
        qDebug()<<"open failed";
    }
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
    file.close();

    //固定窗口大小
    ui->setupUi(this);
    this->setFixedSize(this->size());

    initial();
}

mainDlg::~mainDlg()
{
    qDebug() << "start destroy widget";
    if(m_objThread)
    {
        qDebug() << "quit";
        m_objThread->quit();
        m_objThread->wait();
    }
    qDebug() << "end destroy widget";
    delete ui;
}

void mainDlg::startObjThread()
{
    if(m_objThread)
    {
        return;
    }
    m_objThread= new QThread();
    m_acqThread = new AcquisitionThread(&m_xiCam);
    m_acqThread->moveToThread(m_objThread);
    connect(m_objThread,&QThread::finished,m_objThread,&QObject::deleteLater);
    connect(m_objThread,&QThread::finished,m_acqThread,&QObject::deleteLater);
    connect(this,SIGNAL(startObjThreadWork1()),m_acqThread,SLOT(getImage()));
    connect(m_acqThread,SIGNAL(sendImage(cv::Mat&)),this,SLOT(showImage(cv::Mat&)),Qt::DirectConnection);

    //录制
    connect(this,SIGNAL(saveImage()),m_acqThread,SLOT(saveImageToFile()),Qt::DirectConnection);
    connect(this,SIGNAL(stopSaveImage()),m_acqThread,SLOT(stopSaveImageToFile()),Qt::DirectConnection);
    m_objThread->start();
    qDebug()<<"ok";
}

void mainDlg::on_test_clicked()
{
    QDateTime local(QDateTime::currentDateTime());
    QString localTime = local.toString("yyyyMMddhhmmss");
    qDebug()<<localTime;
    QDir dir;
    if(!dir.exists(localTime))//判断需要创建的文件夹是否存在
    {
        dir.mkdir(localTime); //创建文件夹
    }
    int num = 0;
    QFile file(".//"+localTime + "//" + "file"+ QString::number(num)+".txt");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }
    file.write("11111");
    file.close();
}
void mainDlg::initial()
{    
    int num = m_xiCam.GetNumberOfConnectedCameras();
    if(num > 0){
        QString str;
        for(int i = 0;i<num;i++){
            str = "Cam "+QString::number(num);
            ui->comDeviceNum->addItem(str);
        }
        m_xiCam.OpenByID(ui->comDeviceNum->currentIndex());
        m_xiCam.SetExposureTime(10000);
        m_xiCam.SetAcquisitionTimingMode(XI_ACQ_TIMING_MODE_FRAME_RATE);//设置帧率模式
        m_xiCam.SetWidth(800);
        m_xiCam.SetHeight(600);
        m_xiCam.SetFrameRate(60.0f);
        DEVICE_INFO info = getDevInfo(m_xiCam);
        writeDevParaToXML(m_xiCam);
        //曝光时间()
        ui->spinExposureTime->setValue(info.setting.exposureTime);
        ui->spinExposureTime->setRange(info.miniExposureTime,info.maxExposureTime);//限制最大曝光时间为500000（设备实际最大为1000000）
        ui->spinExposureTime->setSingleStep(10000);
        ui->sliderExposureTime->setValue(info.setting.exposureTime);
        ui->sliderExposureTime->setRange(info.miniExposureTime,info.maxExposureTime);
        ui->sliderExposureTime->setSingleStep(info.exposureTimeIncrement);
        connect(ui->spinExposureTime,SIGNAL(valueChanged(int)),this,SLOT(onExposureTimeChanged(int)));
        connect(ui->sliderExposureTime,SIGNAL(valueChanged(int)),this,SLOT(onExposureTimeChanged(int)));
        //帧率
        ui->leFrameRate->setText(QString::number(info.setting.frameRate));
        connect(ui->leFrameRate,SIGNAL(textChanged(QString)),this,SLOT(onFrameRateChanged(QString)));
        //宽、高
        //COMMbox
        ui->comDPI->addItem("800 x 600 (suggesstion)");
        ui->comDPI->addItem("1280 x 1024");
        ui->comDPI->addItem("1280 x 768");
        ui->comDPI->addItem("900 x 768");
        ui->comDPI->addItem("640 x 512");
        ui->comDPI->addItem("400 x 300");
        QLineEdit* l = new QLineEdit();
        l->setText(ui->comDPI->currentText());
        l->setAlignment(Qt::AlignHCenter);
        ui->comDPI->setLineEdit(l);
        connect(ui->comDPI,SIGNAL(currentTextChanged(QString)),this,SLOT(onDPIChanged(QString)));
//        //触发源
//        ui->comTriSource->addItem(tr("关闭"));
//        ui->comTriSource->addItem(tr("上升沿"));
//        ui->comTriSource->addItem(tr("下降沿"));
//        ui->comTriSource->addItem(tr("软件"));
//        ui->comTriSource->addItem(tr("高电平"));
//        ui->comTriSource->addItem(tr("低电平"));
        //触发方式

        //开启采集线程
        if(!m_objThread)
        {
            startObjThread();
        }
        emit startObjThreadWork1();
    }
}

//获取设备参数信息
DEVICE_INFO mainDlg::getDevInfo(xiAPIplus_Camera& cam)
{
    DEVICE_INFO info;
    info.setting.exposureTime = cam.GetExposureTime();
    info.miniExposureTime = cam.GetExposureTime_Minimum();
    info.maxExposureTime = cam.GetExposureTime_Maximum();
    info.exposureTimeIncrement = cam.GetExposureTime_Increment();

    info.setting.frameRate = cam.GetFrameRate();
    info.miniFrameRate = cam.GetFrameRate_Minimum();
    info.maxFrameRate = cam.GetFrameRate_Maximum();
    info.frameRateIncreament = cam.GetFrameRate_Increment();
    return info;
}

void mainDlg::on_open_clicked()
{
    if(m_xiCam.IsAcquisitionActive()){
        m_xiCam.StopAcquisition();
        m_xiCam.Close();
    }
    initial();
}

void mainDlg::on_record_clicked()
{
    qDebug()<<"on_record_clicked";
    if(m_bIsRecording){
        emit stopSaveImage();
        m_bIsRecording = false;
        ui->record->setText(tr("record"));
    }else{
        emit saveImage();
        m_bIsRecording = true;
        ui->record->setText(tr("stop"));
    }
}

void mainDlg::on_replay_clicked()
{
    qDebug()<<"on_replay_clicked";
    t = new playercontrol(this);
    t->move(0,430);
    t->show();
    connect(this,SIGNAL(test(QLabel*)),t,SLOT(openPlayer(QLabel*)));
    emit test(ui->label);
}

void mainDlg::on_close_clicked()
{
    qDebug()<<"on_close_clicked";
    if(m_objThread)
    {
        if(m_acqThread)
        {
            m_acqThread->stop();
        }
    }
    if(m_xiCam.IsAcquisitionActive()){
        m_xiCam.StopAcquisition();
        m_xiCam.Close();
    }
    ui->label->clear();
}

//接收并显示采集线程传来的图像数据
void mainDlg::showImage(cv::Mat& image)
{

    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setPixmap(QPixmap::fromImage(Mat2QImage(image)));
}

//Mat转为QImage
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

//从XML文件中读取设备的参数信息
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

//将设备的参数信息保存至XML文件中
void mainDlg::writeDevParaToXML(xiAPIplusCameraOcv& cam)
{
    QFile file("XiMeaCamearaConfig.xml");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
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
    xmlWriter.writeTextElement("curExposureTime",QString::number(cam.GetExposureTime()));
    xmlWriter.writeTextElement("miniExposureTime",QString::number(cam.GetExposureTime_Minimum()));
    xmlWriter.writeTextElement("maxExposureTime",QString::number(cam.GetExposureTime_Maximum()));
    xmlWriter.writeTextElement("exposureTimeIncrement",QString::number(cam.GetExposureTime_Increment()));
    xmlWriter.writeEndElement();

//    //光圈(float)
//    xmlWriter.writeStartElement("lensAperture");
//    xmlWriter.writeTextElement("curLensAperture",QString::number(cam.GetLensApertureValue()));
//    xmlWriter.writeTextElement("miniLensAperture",QString::number(cam.GetLensApertureValue_Minimum()));
//    xmlWriter.writeTextElement("maxLensAperture",QString::number(cam.GetLensApertureValue_Maximum()));
//    xmlWriter.writeTextElement("lensApertureIncrement",QString::number(cam.GetLensApertureValue_Increment()));
//    xmlWriter.writeEndElement();

    //帧率(float)
    xmlWriter.writeStartElement("frameRate");
    xmlWriter.writeTextElement("curFrameRate",QString::number(cam.GetFrameRate()));
    xmlWriter.writeTextElement("miniFrameRate",QString::number(cam.GetFrameRate_Minimum()));
    xmlWriter.writeTextElement("maxFrameRate",QString::number(cam.GetFrameRate_Maximum()));
    xmlWriter.writeTextElement("frameRateIncrement",QString::number(cam.GetFrameRate_Increment()));
    xmlWriter.writeEndElement();

//    //焦距（mm）
//    xmlWriter.writeStartElement("focusDistance");
//    xmlWriter.writeTextElement("curFocusDistance",QString::number(cam.GetLensFocalLength()));
//    xmlWriter.writeTextElement("miniFocusDistance",QString::number(cam.GetLensFocalLength_Minimum()));
//    xmlWriter.writeTextElement("maxFocusDistance",QString::number(cam.GetLensFocalLength_Maximum()));
//    xmlWriter.writeTextElement("focusDistanceIncrement",QString::number(cam.GetLensFocalLength_Increment()));
//    xmlWriter.writeEndElement();


    //图像宽度（像素int）
    xmlWriter.writeStartElement("widthInfo");
    xmlWriter.writeTextElement("curWidth",QString::number(cam.GetWidth()));
    xmlWriter.writeTextElement("miniWidth",QString::number(cam.GetWidth_Minimum()));
    xmlWriter.writeTextElement("maxWidth",QString::number(cam.GetWidth_Maximum()));
    xmlWriter.writeTextElement("widthIncrement",QString::number(cam.GetWidth_Increment()));
    xmlWriter.writeEndElement();

    //图像高度（像素int）
    xmlWriter.writeStartElement("sizeInfo");
    xmlWriter.writeTextElement("curHeight",QString::number(cam.GetHeight()));
    xmlWriter.writeTextElement("miniHeight",QString::number(cam.GetHeight_Minimum()));
    xmlWriter.writeTextElement("maxHeight",QString::number(cam.GetHeight_Maximum()));
    xmlWriter.writeTextElement("heightIncrement",QString::number(cam.GetHeight_Increment()));
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();        //XiMea Cameara
    xmlWriter.writeEndElement();        //相机参数
    xmlWriter.writeEndDocument();

    file.close();
}

//响应关闭事件
void mainDlg::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    button=QMessageBox::question(this,tr("exit"),QString(tr("Are you sure to leave me?")),QMessageBox::Yes|QMessageBox::No);
    if(button==QMessageBox::No)
    {
        event->ignore(); // 忽略退出信号，程序继续进行
    }
    else if(button==QMessageBox::Yes)
    {
        event->accept(); // 接受退出信号，程序退出
        if(m_objThread)
        {
            if(m_acqThread)
            {
                m_acqThread->stop();
            }
        }
        if(m_xiCam.IsAcquisitionActive()){
            m_xiCam.StopAcquisition();
            m_xiCam.Close();
        }
    }
}

//界面函数，响应曝光时间
void mainDlg::onExposureTimeChanged(int value)
{
    ui->spinExposureTime->setValue(value);
    ui->sliderExposureTime->setValue(value);
    m_xiCam.SetExposureTime(value);
}

void mainDlg::onFrameRateChanged(QString value)
{
    float val = value.toFloat();
    if(val - m_xiCam.GetFrameRate_Minimum() < 0.00001f){
       m_xiCam.SetFrameRate(m_xiCam.GetFrameRate());
       ui->leFrameRate->setText(QString::number(m_xiCam.GetFrameRate_Minimum()));
    }
    else if(m_xiCam.GetFrameRate_Maximum() - val < 0.00001f){
       m_xiCam.SetFrameRate(m_xiCam.GetFrameRate_Maximum());
       ui->leFrameRate->setText(QString::number(m_xiCam.GetFrameRate_Maximum()));
    }
    else{
        m_xiCam.SetFrameRate(val);
    }
}

void mainDlg::onDPIChanged(QString value)
{
    QStringList list = value.split(tr(" "));
    QString temp = list[0];
    qDebug()<<temp.toInt();
    m_xiCam.SetWidth(temp.toInt());
    temp = list[2];
    qDebug()<<temp.toInt();
    m_xiCam.SetHeight(temp.toInt());
}

