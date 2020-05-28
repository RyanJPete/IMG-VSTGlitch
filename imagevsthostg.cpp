#include "imagevsthostg.h"
#include "ui_imagevsthostg.h"
#include "ImageVSTHost.h"
#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QLabel>
#include <QScrollArea>

//#include <opencv2/core.hpp>

ImageVSTHostG::ImageVSTHostG(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageVSTHostG)
{
    hst = new Host();
    hst->initializeIO();
    char vstPath[] = "C:\\VSTs\\DragonflyRoomReverb-vst.dll";
    //char vstPath[] = "C:\\VSTs\\CocoaDelay64";
    plug = hst->loadPlugin(vstPath);
    hst->configurePluginCallbacks(plug);
    hst->startPlugin(plug);
    ui->setupUi(this);

    displayPreview("img.bmp");


    QVBoxLayout *controlLay = new QVBoxLayout();

    for(int x = 0; x < plug->numParams; x++){
        addSlider(controlLay, hst, plug, x);
    }

    QFrame *controlFrame = new QFrame(ui->controlArea);
    controlFrame->setLayout(controlLay);
    ui->controlArea->setWidget(controlFrame);

}

ImageVSTHostG::~ImageVSTHostG()
{
    delete ui;
}

int ImageVSTHostG::VSTFloatToInt(float val){
    int ret = val*100;          //aproximation, data will be lost and I will revise if it turns out to be needed
    if(ret == 100){             //stay in default range
        ret = 99;
    }
    return ret;
}

float ImageVSTHostG::intToVSTFloat(int val){
    float ret = val;
    ret/=100;
    return ret;
}

void ImageVSTHostG::displayPreview(QString path){
    QLabel *imageLabel = new QLabel;
    QImage image(path);
    imageLabel->setPixmap(QPixmap::fromImage(image));

    // area->setWidgetResizable(true);
    ui->previewArea->setMinimumSize(500,500);
    ui->previewArea->setBackgroundRole(QPalette::Mid);
    ui->previewArea->setWidget(imageLabel);
}


void ImageVSTHostG::addSlider(QVBoxLayout *box, Host *hst, AEffect *plug, int plugNum){
    QString labText = QString::fromStdString(hst->getParamName(plug, plugNum));
    QLabel *lab = new QLabel(labText);
    QSlider *slid = new QSlider(Qt::Horizontal);

    connect(slid, &QSlider::valueChanged, [=] () {          //change vst param value
        plug->setParameter(plug, plugNum, intToVSTFloat(slid->value()));
        std::cout << slid->value() <<"," <<VSTFloatToInt(plug->getParameter(plug,plugNum)) << std::endl;
        procAndDisplay();
    });

    int slidVal = VSTFloatToInt(plug->getParameter(plug,plugNum));
    slid->setValue(slidVal);
    box->addWidget(lab);
    box->addWidget(slid);
}

void ImageVSTHostG::procAndDisplay(){
    hst->loadImage("C:\\Users\\Jerome\\Documents\\Programming\\ImageVSTHostG\\happyguy.bmp");
    hst->processAudio(plug);
    hst->copyHeader();
    hst->writeOutputs("output.bmp");
//    displayPreview("C:\\Users\\Jerome\\Documents\\Programming\\ImageVSTHostG\\happyguy.bmp");
    displayPreview("C:\\Users\\Jerome\\Documents\\Programming\\build-ImageVSTHostG-Desktop_Qt_5_12_6_MinGW_64_bit-Debug\\output.bmp");
}

void ImageVSTHostG::on_runVSTBtn_clicked()
{
   procAndDisplay();
}
