#pragma once
#ifndef QUADQTVTKRENDERWIDGET_H
#define QUADQTVTKRENDERWIDGET_H

#include <QFileInfo>
#include <QWidget>
#include <vtkSmartPointer.h>
class vtkDistanceWidget;
class vtkImagePlaneWidget;
class vtkNrrdReader;
class vtkResliceImageViewer;
class vtkResliceImageViewerMeasurements;
class QVTKOpenGLNativeWidget;

class QuadQtVTKRenderWidget : public QWidget
{
    Q_OBJECT

public:
    QuadQtVTKRenderWidget(QWidget* parent = Q_NULLPTR);
    ~QuadQtVTKRenderWidget() override = default;
    void setVolumeImageFile(const QFileInfo& fileName);

    // public slots:

protected:
    QList<QVTKOpenGLNativeWidget*> m_vtkWidgets = { nullptr, nullptr, nullptr, nullptr };
    vtkSmartPointer<vtkResliceImageViewer> m_riw[3];
    vtkSmartPointer<vtkImagePlaneWidget> m_planeWidget[3];
    vtkSmartPointer<vtkDistanceWidget> m_distanceWidget[3];
    vtkSmartPointer<vtkResliceImageViewerMeasurements> m_resliceMeasurements;

    vtkSmartPointer<vtkNrrdReader> m_reader;
};

#endif
