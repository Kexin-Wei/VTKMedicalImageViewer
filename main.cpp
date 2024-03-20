#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <QVTKOpenGLWidget.h>
#include <QVTKOpenGLWindow.h>
#include <QWidget>

#include <vtkSmartPointer.h>

// #include <src/QuadQtVTKRenderWidget.h>
// #include <src/VTKOpenGLWidget.h>
// #include "QVTKOpenGLNativeWidget.h"
#include <lib/QuadViewerWidget.h>
#include <lib/VolumeImage.h>
using QuadViewerWidget = ultrast::visualization::QuadViewerWidget;
using VolumeImage = ultrast::infrastructure::utility::VolumeImage;
using VolumeImageType = ultrast::infrastructure::utility::VolumeImageType;
int main(int argc, char* argv[])
{
    // QSurfaceFormat::setDefaultFormat(VTKOpenGLWidget::defaultFormat());

    QApplication app(argc, argv);
    // QMainWindow w;
    // QuadQtVTKRenderWidget* renderWidget = new QuadQtVTKRenderWidget(&w);
    // w.setCentralWidget(renderWidget);
    // w.show();
    // VTKOpenGLWidget widget;
    // widget.show();
    QWidget parent;
    parent.resize(1200, 600);
    QHBoxLayout* layout = new QHBoxLayout(&parent);
    parent.setLayout(layout);

    QuadViewerWidget usWidget(&parent);
    QuadViewerWidget mriWidget(&parent);
    layout->addWidget(&usWidget);
    layout->addWidget(&mriWidget);

    /**********************************************/
    QString usFilePath = "D:/US.mha";
    QString mriFilePath = "D:/MRI.nrrd";
    /**********************************************/

    VolumeImage* usImage
        = new VolumeImage(usFilePath, "US", VolumeImageType::ULTRASOUND);
    auto imageData = usImage->getImageData();
    VolumeImage* mriImage
        = new VolumeImage(mriFilePath, "MRI", VolumeImageType::MRI);
    { //set up us widget
        usWidget.setViewerLayout(QuadViewerWidget::ViewerLayout::REGISTRATION);

        double imageCenter[3];
        imageData->GetCenter(imageCenter);
        double bounds[6];
        usImage->getBounds(bounds);
        usImage->setWorldRotationCenter(imageCenter);
        auto usCenter = usImage->getWorldRotationCenter();

        usWidget.addData(*usImage);
        usWidget.resetCameraAndSetCrosshair(bounds, usCenter);
    }

    { //set up mri widget
        mriWidget.setViewerLayout(QuadViewerWidget::ViewerLayout::REGISTRATION);

        auto imageData = mriImage->getImageData();
        double imageCenter[3];
        imageData->GetCenter(imageCenter);
        double bounds[6];
        mriImage->getBounds(bounds);
        mriImage->setWorldRotationCenter(imageCenter);
        auto mriCenter = mriImage->getWorldRotationCenter();

        mriWidget.addData(*mriImage);
        mriWidget.resetCameraAndSetCrosshair(bounds, mriCenter);
    }

    { // pre-registration, move mri image to us
    }
    QObject::connect(&usWidget, &QuadViewerWidget::coordinateChanged,
        &mriWidget, &QuadViewerWidget::setCrosshairCoordinate);
    QObject::connect(&mriWidget, &QuadViewerWidget::coordinateChanged,
        &usWidget, &QuadViewerWidget::setCrosshairCoordinate);

    parent.show();
    return app.exec();
}

