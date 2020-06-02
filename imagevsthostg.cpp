#include "imagevsthostg.h"
#include "ui_imagevsthostg.h"
#include "ImageVSTHost.h"
#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QLabel>
#include <QScrollArea>
#include <QFileDialog>

//#include <opencv2/core.hpp>

ImageVSTHostG::ImageVSTHostG(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageVSTHostG)
{
    char vstPath[] = "C:\\VSTs\\DragonflyRoomReverb-vst.dll";
    //char vstPath[] = "C:\\VSTs\\CocoaDelay64";


    ui->setupUi(this);
    connect(ui->actionLoad_VST, &QAction::triggered, [=] () {
       changeVST();
    });
    connect(ui->actionLoadImage, &QAction::triggered, [=] () {          //change vst param value
        imgFilePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "C:\\", tr("Image Files (*.png *.jpg *.bmp)"));
    });

    imgFilePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "C:\\", tr("Image Files (*.png *.jpg *.bmp)"));

    displayPreview(imgFilePath);

    controlLay = new QVBoxLayout();

    QFrame *controlFrame = new QFrame(ui->controlArea);
    controlFrame->setLayout(controlLay);
    ui->controlArea->setWidget(controlFrame);


}

ImageVSTHostG::~ImageVSTHostG()
{
    delete ui;
}

void ImageVSTHostG::changeVST(){
    char *vstPath;
    QString tempstr = QFileDialog::getOpenFileName(this, tr("Open Image"), "C:\\", tr("VSTs (*.dll)"));
    QByteArray tempba = tempstr.toLatin1();
    vstPath = tempba.data();
    hst = new Host();
    hst->initializeIO();
    plug = hst->loadPlugin(vstPath);
    hst->configurePluginCallbacks(plug);
    hst->startPlugin(plug);
    procAndDisplay();

    //delete old controlls
    QLayoutItem *child;
    while((child = controlLay->takeAt(0)) != 0){
        delete child->widget();
       // delete child;
    }

    //add controlls

    for(int x = 0; x < plug->numParams; x++){
        addSlider(controlLay, hst, plug, x);
    }


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
        //std::cout << slid->value() <<"," <<VSTFloatToInt(plug->getParameter(plug,plugNum)) << std::endl;
        procAndDisplay();
    });

    int slidVal = VSTFloatToInt(plug->getParameter(plug,plugNum));
    slid->setValue(slidVal);
    box->addWidget(lab);
    box->addWidget(slid);
}

void ImageVSTHostG::procAndDisplay(){
    hst->loadImage(imgFilePath.toStdString());
    hst->processAudio(plug);
    hst->copyHeader();
    hst->writeOutputs("output.bmp");
    displayPreview("output.bmp");
}

void ImageVSTHostG::on_runVSTBtn_clicked()
{
   procAndDisplay();
}

