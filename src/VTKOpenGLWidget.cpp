#include "VTKOpenGLWidget.h"

#include <QDebug>

#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageReslice.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkNrrdReader.h>
#include <vtkOutlineFilter.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>

VTKOpenGLWidget::VTKOpenGLWidget(QWidget* parent) :
    QVTKOpenGLNativeWidget(parent),
    m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New()),
    m_leftRenderer(vtkSmartPointer<vtkRenderer>::New()),
    m_rightRenderer(vtkSmartPointer<vtkRenderer>::New()),
    m_origin { 50, 50, 0 }
{
    initialize();
    createTestData();
}

VTKOpenGLWidget::~VTKOpenGLWidget()
{
}

void VTKOpenGLWidget::initialize()
{
    m_renderWindow->AddRenderer(m_leftRenderer);
    m_leftRenderer->SetBackground(0.5, 0.5, 0.5);
    m_leftRenderer->SetViewport(0, 0, 0.5, 1);
    m_renderWindow->AddRenderer(m_rightRenderer);
    m_rightRenderer->SetBackground(0, 1.0, 0);
    m_rightRenderer->SetViewport(0.5, 0, 1, 1);
    SetRenderWindow(m_renderWindow);
}

void VTKOpenGLWidget::createTestData()
{
    createLeftRenderData();
    createRightRenderData();
}

void VTKOpenGLWidget::createLeftRenderData()
{
    vtkNew<vtkNrrdReader> reader;
    reader->SetFileName("D:/MRI.nrrd");

    vtkNew<vtkImageResliceMapper> imageResliceMapper;
    imageResliceMapper->SetInputConnection(reader->GetOutputPort());

    reader->Update();
    auto imageCenter = reader->GetOutput()->GetCenter();
    double normalZ[] = { 0, 0, 1 };
    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(imageCenter);
    plane->SetNormal(normalZ);

    imageResliceMapper->SetSlicePlane(plane);

    vtkNew<vtkImageSlice> imageSlice;
    imageSlice->SetMapper(imageResliceMapper);

    this->m_leftRenderer->AddActor(imageSlice);
}

void VTKOpenGLWidget::createRightRenderData()
{
    vtkNew<vtkNrrdReader> reader;
    reader->SetFileName("D:/MRI.nrrd");

    vtkNew<vtkImageResliceMapper> imageResliceMapper;
    imageResliceMapper->SetInputConnection(reader->GetOutputPort());

    reader->Update();
    auto imageCenter = reader->GetOutput()->GetCenter();
    qDebug() << imageCenter[0] << imageCenter[1] << imageCenter[2];
    double normalZ[] = { 0, 0, 1 };
    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(imageCenter);
    plane->SetNormal(normalZ);

    double newLoc[] = { imageCenter[0], imageCenter[1], imageCenter[2] + 10 };
    plane->SetOrigin(newLoc);
    imageResliceMapper->SetSlicePlane(plane);

    vtkNew<vtkImageSlice> imageSlice;
    imageSlice->SetMapper(imageResliceMapper);

    this->m_rightRenderer->AddActor(imageSlice);
}
