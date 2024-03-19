#pragma once

#include <QVTKOpenGLNativeWidget.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>

class vtkGenericOpenGLRenderWindow;
class vtkRenderer;
class vtkTransform;
class SliceViewerInteractorStyle : public vtkInteractorStyleImage
{
public:
static SliceViewerInteractorStyle* New();
    ~SliceViewerInteractorStyle();
    void OnMouseMove() override;
    void OnLeftButtonDown() override;

private:
    SliceViewerInteractorStyle();
    vtkRenderer* m_renderer;
    vtkRenderWindow* m_renderWindow;
};

class VTKOpenGLWidget : public QVTKOpenGLNativeWidget
{
public:
    VTKOpenGLWidget(QWidget* parent = nullptr);
    ~VTKOpenGLWidget();

private:
    void initialize();
    void setUpImages();
    void createFirsImageRenderData();
    void createSecondImageRenderData();

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    std::vector<vtkSmartPointer<vtkRenderer>> m_firstRenderer;
    vtkSmartPointer<vtkRenderer> m_rightRenderer;
    vtkSmartPointer<vtkTransform> m_firstTransform;
    vtkSmartPointer<vtkRenderer> m_leftRenderer;
    std::map<int, std::vector<double>> m_normals;
    double m_cursor[3];
};