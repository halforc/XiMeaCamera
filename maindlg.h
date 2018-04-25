#ifndef MAINDLG_H
#define MAINDLG_H

#include <QDialog>
#include "typedefine.h"
#include <opencv2/imgproc/imgproc.hpp>

#include <QPixmap>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include "acquisitionthread.h"
#include "playercontrol.h"
namespace Ui {
class mainDlg;
}

class mainDlg : public QDialog
{
    Q_OBJECT

public:
    explicit mainDlg(QWidget *parent = 0);
    ~mainDlg();

public:
    cv::Mat cv_mat_image;
    QImage img;
    QPixmap pixmap;
    xiAPIplusCameraOcv m_xiCam;

    QTimer* timer;

    AcquisitionThread* m_acqThread;
    QThread* m_objThread;

    void startObjThread();
public slots:
    void on_test_clicked();
    void on_open_clicked();
    void on_record_clicked();
    void on_replay_clicked();
    void on_close_clicked();

    void showImage(cv::Mat& image);

    void onExposureTimeChanged(int value);
    void onFrameRateChanged(QString value);
    void onDPIChanged(QString value);

signals:
    void test(QLabel* label);
    void startObjThreadWork1();
    void saveImage();
    void stopSaveImage();
private:
    Ui::mainDlg *ui;
    playercontrol* t;
    QImage Mat2QImage(cv::Mat cvImg);
    void initial();
    bool m_bIsRecording;
    DEVICE_INFO getDevInfo(xiAPIplus_Camera& cam);

    void readDevParaFromXML(DEVICE_SETTING *pDevInfo);
    void writeDevParaToXML(xiAPIplusCameraOcv &cam);

    void closeEvent(QCloseEvent* event);
};

#endif // MAINDLG_H
