#ifndef MAINDLG_H
#define MAINDLG_H

#include <QDialog>
#include "./XiAPI/xiApiPlusOcv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QPixmap>
#include <QTimer>
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
    void paintEvent(QPaintEvent *event);
    cv::Mat cv_mat_image;
    QImage Mat2QImage(cv::Mat cvImg);
    QImage img;

    QTimer* mTimer;
public slots:
    void on_test_clicked();
    void onTimeOut();
private:
    Ui::mainDlg *ui;
};

#endif // MAINDLG_H
