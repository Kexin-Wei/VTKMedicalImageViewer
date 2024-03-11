
#include "QuadQtVTKRenderWidget.h"

#include <QGridLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <QWidget>

#include <vtkCellPicker.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageMapToColors.h>
#include <vtkImagePlaneWidget.h>
#include <vtkInteractorStyleImage.h>
#include <vtkNrrdReader.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceImageViewer.h>
#include "vtkResliceCursorCallback.h"

QuadQtVTKRenderWidget::QuadQtVTKRenderWidget(QWidget* parent) :
    QWidget(parent)
{
    resize(851, 583);
    setWindowTitle("QuadQtVTKRenderWidget");

    m_reader = vtkSmartPointer<vtkNrrdReader>::New();
    m_reader->SetFileName("D:/Medical Image - Example/Real-Patient-Data/Patient H/MR_t2_tse_tra_p2-4mm-wwp_FIL.nrrd");

    QGridLayout* gridLayout = new QGridLayout(this);
    for (auto vtkWidget : m_vtkWidgets)
    {
        vtkWidget = new QVTKOpenGLNativeWidget(this);
    }
    gridLayout->addWidget(m_vtkWidgets[0], 0, 0, 1, 1);
    gridLayout->addWidget(m_vtkWidgets[1], 1, 0, 1, 1);
    gridLayout->addWidget(m_vtkWidgets[2], 1, 1, 1, 1);
    gridLayout->addWidget(m_vtkWidgets[3], 0, 1, 1, 1); // stereo view

    for (int i = 0; i < 3; i++)
    {
        m_riw[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
        vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        m_riw[i]->SetRenderWindow(renderWindow);
        m_riw[i]->SetupInteractor(m_vtkWidgets[i]->GetInteractor());
        m_riw[i]->SetSliceOrientation(i);
        vtkSmartPointer<vtkResliceCursorLineRepresentation> rep = vtkSmartPointer<vtkResliceCursorLineRepresentation>::New();
        rep->SafeDownCast(m_riw[i]->GetResliceCursorWidget()->GetRepresentation());
        m_riw[i]->SetResliceCursor(m_riw[0]->GetResliceCursor());
        rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

        m_riw[i]->SetInputConnection(m_reader->GetOutputPort());
        m_riw[i]->SetResliceModeToAxisAligned();
    }

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> rcw = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_vtkWidgets[3]->SetRenderWindow(rcw);
    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    m_vtkWidgets[3]->GetRenderWindow()->AddRenderer(ren);

    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);
    vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();
    for (int i = 0; i < 3; i++)
    {
        m_planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        m_planeWidget[i]->SetInteractor(m_vtkWidgets[3]->GetInteractor());
        m_planeWidget[i]->SetPicker(picker);
        m_planeWidget[i]->RestrictPlaneToVolumeOn();
        double color[3] = { 0, 0, 0 };
        color[i] = 1;
        m_planeWidget[i]->GetPlaneProperty()->SetColor(color);
        color[0] /= 4.0;
        color[1] /= 4.0;
        color[2] /= 4.0;
        m_riw[i]->GetRenderer()->SetBackground(color);

        m_planeWidget[i]->SetTexturePlaneProperty(ipwProp);
        m_planeWidget[i]->TextureInterpolateOff();
        m_planeWidget[i]->SetResliceInterpolateToLinear();
        m_planeWidget[i]->SetInputConnection(m_reader->GetOutputPort());
        m_planeWidget[i]->SetPlaneOrientation(i);
        m_planeWidget[i]->SetSliceIndex(0);
        m_planeWidget[i]->DisplayTextOn();
        m_planeWidget[i]->SetDefaultRenderer(ren);
        m_planeWidget[i]->SetWindowLevel(1358, -27);
        m_planeWidget[i]->On();
        m_planeWidget[i]->InteractionOn();
    }

    vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();

    for (int i = 0; i < 3; i++)
    {
        cbk->IPW[i] = m_planeWidget[i];
        cbk->RCW[i] = m_riw[i]->GetResliceCursorWidget();
        m_riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
        m_riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
        m_riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
        m_riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
        m_riw[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, cbk);
        m_riw[i]->GetInteractorStyle()->AddObserver(vtkResliceImageViewer::SliceChangedEvent, cbk);

        m_riw[i]->SetLookupTable(m_riw[0]->GetLookupTable());
        m_planeWidget[i]->GetColorMap()->SetLookupTable(m_riw[0]->GetLookupTable());
        m_planeWidget[i]->SetColorMap(m_riw[0]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
    }

    m_vtkWidgets[0]->show();
    m_vtkWidgets[1]->show();
    m_vtkWidgets[2]->show();
}

void QuadQtVTKRenderWidget::setVolumeImageFile(const QFileInfo& fileName)
{
    m_reader->SetFileName(fileName.filePath().toStdString().c_str());
    m_reader->Update();
    // TODO: add code to display the volume image
}
