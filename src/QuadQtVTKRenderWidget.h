#pragma once
#ifndef QUADQTVTKRENDERWIDGET_H
#define QUADQTVTKRENDERWIDGET_H

#include <QFileInfo>
#include <QWidget>
#include <array>
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
    std::array<vtkSmartPointer<vtkResliceImageViewer>, 3> m_riw;
    std::array<vtkSmartPointer<vtkImagePlaneWidget>, 3> m_planeWidget;
    std::array<vtkSmartPointer<vtkDistanceWidget>, 3> m_distanceWidget;

    vtkSmartPointer<vtkResliceImageViewerMeasurements> m_resliceMeasurements;

    vtkSmartPointer<vtkNrrdReader> m_reader;
};

#endif
