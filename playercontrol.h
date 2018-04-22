#ifndef PLAYERCONTROL_H
#define PLAYERCONTROL_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class playercontrol;
}

class playercontrol : public QWidget
{
    Q_OBJECT

public:
    explicit playercontrol(QWidget *parent = 0);
    ~playercontrol();

private:
    Ui::playercontrol *ui;

public slots:
    void openPlayer(QLabel* label);
};

#endif // PLAYERCONTROL_H
