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
    std::vector<vtkSmartPointer<vtkRenderer>> createViewForOneImage(double ymin, double ymax, char* fileName);

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_rightRenderer;
    vtkSmartPointer<vtkTransform> m_firstTransform;
    vtkSmartPointer<vtkRenderer> m_leftRenderer;
    std::map<int, std::vector<double>> m_normals;
    std::map<int, std::vector<double>> m_viewUps;
    double m_cursor[3];
};