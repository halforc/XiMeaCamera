#ifndef MAINDLG_H
#define MAINDLG_H

#include <QDialog>
#include "typedefine.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QPixmap>
#include <QThread>
#include <QDebug>

#include "playercontrol.h"
namespace Ui {
class mainDlg;
}

class Worker : public QThread
{
    Q_OBJECT
private:
    void run()
    {
        mStart = true;
        while(mStart){
            qDebug()<<"Worker::onTimeout get called from?: "<<QThread::currentThreadId();
            sleep(1);
        }
    }

public:

    void stop(){
        mStart = false;
    }

private:
    bool mStart;
};

class mainDlg : public QDialog
{
    Q_OBJECT

public:
    explicit mainDlg(QWidget *parent = 0);
    ~mainDlg();

public:
    void paintEvent(QPaintEvent *event);
    cv::Mat cv_mat_image;
    QImage img;

    xiAPIplusCameraOcv m_xiCam;
public slots:
    void on_test_clicked();
    void on_open_clicked();
    void on_record_clicked();
    void on_replay_clicked();
    void on_close_clicked();

signals:
    void test(QLabel* label);

private:
    Ui::mainDlg *ui;
    playercontrol* t;
    QImage Mat2QImage(cv::Mat cvImg);
    void initial();

    void readDevParaFromXML(DEVICE_SETTING *pDevInfo);
    void writeDevParaToXML(DEVICE_INFO* pDevInfo);
    bool eventFilter(QObject *, QEvent *);
};

#endif // MAINDLG_H
