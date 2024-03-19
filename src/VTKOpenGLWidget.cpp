#include "VTKOpenGLWidget.h"

#include <QDebug>

#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageReslice.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkNrrdReader.h>
#include <vtkOutlineFilter.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>

SliceViewerInteractorStyle* SliceViewerInteractorStyle::New()
{
    return new SliceViewerInteractorStyle;
}

SliceViewerInteractorStyle::SliceViewerInteractorStyle() :
    vtkInteractorStyleImage(),
    m_renderer(nullptr),
    m_renderWindow(nullptr)
{
}

SliceViewerInteractorStyle::~SliceViewerInteractorStyle()
{
}

void SliceViewerInteractorStyle::OnMouseMove()
{
    qDebug() << "Mouse move";
    int x, y;
    GetInteractor()->GetEventPosition(x, y);
    qDebug() << "Mouse move at " << x << " " << y;
}

void SliceViewerInteractorStyle::OnLeftButtonDown()
{
    int x, y;
    GetInteractor()->GetEventPosition(x, y);
    qDebug() << "Left button down at " << x << " " << y;
}

VTKOpenGLWidget::VTKOpenGLWidget(QWidget* parent) :
    QVTKOpenGLNativeWidget(parent),
    m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New()),
    m_cursor { 0, 0, 0 }
{
    m_normals = { { 0, { 0, 0, 1 } }, { 1, { 0, 1, 0 } }, { 2, { 1, 0, 0 } } };
    m_firstTransform = vtkSmartPointer<vtkTransform>::New();
    resize(1200, 600);
    initialize();
    setUpImages();
}

VTKOpenGLWidget::~VTKOpenGLWidget()
{
}

void VTKOpenGLWidget::initialize()
{
    // auto style = SliceViewerInteractorStyle::New();
    // style->SetInteractor(interactor);
    vtkSmartPointer<vtkNrrdReader> firstReader
        = vtkSmartPointer<vtkNrrdReader>::New();
    firstReader->SetFileName("D:/MRI.nrrd");

    // auto firstCast = vtkSmartPointer<vtkImageCast>::New();
    // firstCast->SetInputConnection(firstReader->GetOutputPort());
    // firstCast->SetOutputScalarTypeToChar();
    // firstCast->ClampOverflowOn();
    // firstCast->Update();
    firstReader->Update();
    auto firstCenter = firstReader->GetOutput()->GetCenter();

    for (int i = 0; i < 3; i++)
    {
        double color[] = { 0, 0, 0 };
        color[i] = 1;
        m_firstRenderer.push_back(vtkSmartPointer<vtkRenderer>::New());
        m_firstRenderer[i]->SetBackground(color);

        vtkSmartPointer<vtkImageResliceMapper> imageResliceMapper
            = vtkSmartPointer<vtkImageResliceMapper>::New();
        // imageResliceMapper->SetInputConnection(firstCast->GetOutputPort());
        imageResliceMapper->SetInputConnection(firstReader->GetOutputPort());

        vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
        plane->SetOrigin(firstCenter);
        plane->SetNormal(m_normals.at(i).data());
        imageResliceMapper->SetSlicePlane(plane);

        vtkSmartPointer<vtkImageSlice> imageSlice
            = vtkSmartPointer<vtkImageSlice>::New();
        auto lookUpTable = vtkSmartPointer<vtkLookupTable>::New();
        // lookUpTable->SetRange(firstCast->GetOutput()->GetScalarRange());
        lookUpTable->SetRange(firstReader->GetOutput()->GetScalarRange());
        lookUpTable->SetValueRange(0.0, 1.0);
        lookUpTable->SetSaturationRange(0.0, 0.0);
        lookUpTable->SetRampToLinear();
        lookUpTable->Build();

        imageSlice->GetProperty()->UseLookupTableScalarRangeOn();
        imageSlice->GetProperty()->SetLookupTable(lookUpTable);
        imageSlice->GetProperty()->SetInterpolationTypeToNearest();

        imageSlice->SetUserTransform(m_firstTransform);

        imageSlice->SetMapper(imageResliceMapper);
        m_firstRenderer[i]->AddActor(imageSlice);
        m_renderWindow->AddRenderer(m_firstRenderer[i]);
    }

    m_firstRenderer[0]->SetViewport(0, 0.5, 0.33, 1);
    m_firstRenderer[1]->SetViewport(0.33, 0.5, 0.66, 1);
    m_firstRenderer[2]->SetViewport(0.66, 0.5, 1, 1);
    // m_renderWindow->AddRenderer(m_leftRenderer);
    // m_leftRenderer->SetBackground(0.5, 0.5, 0.5);
    // m_leftRenderer->SetViewport(0, 0, 0.5, 1);

    // m_renderWindow->AddRenderer(m_rightRenderer);
    // m_rightRenderer->SetBackground(0, 1.0, 0);
    // m_rightRenderer->SetViewport(0.5, 0, 1, 1);
    SetRenderWindow(m_renderWindow);
}

void VTKOpenGLWidget::setUpImages()
{
    // createFirsImageRenderData();
    // createSecondImageRenderData();
}

void VTKOpenGLWidget::createFirsImageRenderData()
{
    vtkNew<vtkNrrdReader> reader;
    reader->SetFileName("D:/MRI.nrrd");

    vtkNew<vtkImageResliceMapper> imageResliceMapper;
    imageResliceMapper->SetInputConnection(reader->GetOutputPort());

    auto imageCenter = reader->GetOutput()->GetCenter();
    qDebug() << imageCenter[0] << imageCenter[1] << imageCenter[2];
    double normalZ[] = { 0, 0, 1 };
    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(imageCenter);
    plane->SetNormal(normalZ);

    imageResliceMapper->SetSlicePlane(plane);

    vtkNew<vtkImageSlice> imageSlice;
    imageSlice->SetMapper(imageResliceMapper);

    this->m_leftRenderer->AddActor(imageSlice);
}

void VTKOpenGLWidget::createSecondImageRenderData()
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
