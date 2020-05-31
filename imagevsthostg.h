#ifndef IMAGEVSTHOSTG_H
#define IMAGEVSTHOSTG_H

#include <QMainWindow>
#include <QVBoxLayout>
#include "ImageVSTHost.h"



QT_BEGIN_NAMESPACE
namespace Ui { class ImageVSTHostG; }
QT_END_NAMESPACE



class ImageVSTHostG : public QMainWindow
{
    Q_OBJECT

public:
    ImageVSTHostG(QWidget *parent = nullptr);
    ~ImageVSTHostG();
    Host *hst;
    AEffect *plug;

private slots:
    void on_runVSTBtn_clicked();

private:
    QString imgFilePath;
    Ui::ImageVSTHostG *ui;
    void addSlider(QVBoxLayout *box, Host *hst, AEffect *plug, int plugNum);
    void displayPreview(QString path);
    int VSTFloatToInt(float val);
    float intToVSTFloat(int val);
    void procAndDisplay();
    void changeVST();

};
#endif // IMAGEVSTHOSTG_H
