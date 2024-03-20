#define _USE_MATH_DEFINES
#include <math.h>

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
        // preregistration
        auto preregistrationTransform = vtkSmartPointer<vtkTransform>::New();
        mriImage->resetTransform();
        preregistrationTransform->Concatenate(mriImage->getWorldTransform());
        // get prostate center in world
        unit::Point usProstateCenterWorld = usImage->getWorldRotationCenter();
        // calculate angle to center line with probe center
        double rotationDegreeByZ = 0;
        auto distanceXY = [](const unit::Point& a, const unit::Point& b) {
            return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
        };
        double allZeros[3] = { 0, 0, 0 };
        unit::Point worldCenter(allZeros);
        const bool POSITIVE = usProstateCenterWorld.x >= worldCenter.x;
        double hypotenuse = distanceXY(usProstateCenterWorld, worldCenter);
        unit::Point intermediary(worldCenter.x, worldCenter.y - hypotenuse,
            worldCenter.z);
        double oppositeSide = distanceXY(usProstateCenterWorld, intermediary);
        rotationDegreeByZ = std::asin(oppositeSide / 2.0 / hypotenuse) * 2
            * 180.0 / M_PI; // solve with sin for half of isosceles triangle
        if (!POSITIVE)
            rotationDegreeByZ *= -1;
        // apply translation
        auto translation = intermediary - mriImage->getWorldRotationCenter();
        preregistrationTransform->Translate(translation.x, translation.y,
            translation.z);
        // apply rotation
        preregistrationTransform->GetPosition();
        translation = allZeros;
        preregistrationTransform->Translate(-translation.x, -translation.y,
            -translation.z);
        preregistrationTransform->RotateZ(rotationDegreeByZ);
        preregistrationTransform->Translate(translation.x, translation.y,
            translation.z);
        // transform secondMain image with secondMain prostate center to us prostate center using probe center as rotation center
        mriImage->setWorldTransform(preregistrationTransform);
    }

    { // sync bounds
        double usBounds[6];
        usImage->getBounds(usBounds);
        double mriBounds[6];
        mriImage->getBounds(mriBounds);
        double bounds[6];
        for (int i = 0; i < 6; ++i)
        {
            if (i % 2 == 0)
                bounds[i] = std::min(usBounds[i], mriBounds[i]);
            else
                bounds[i] = std::max(usBounds[i], mriBounds[i]);
        }
        usWidget.setAllDataBounds(bounds);
        mriWidget.setAllDataBounds(bounds);
    }

    mriWidget.resetCamera();

    QObject::connect(&usWidget, &QuadViewerWidget::coordinateChanged,
        &mriWidget, &QuadViewerWidget::setCrosshairCoordinate);
    QObject::connect(&mriWidget, &QuadViewerWidget::coordinateChanged,
        &usWidget, &QuadViewerWidget::setCrosshairCoordinate);

    parent.show();
    return app.exec();
}

