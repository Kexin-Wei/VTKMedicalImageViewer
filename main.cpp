#include "vtkActor.h"
#include "vtkAutoInit.h"
#include "vtkAxesActor.h"
#include "vtkBiDimensionalWidget.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDICOMImageReader.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkImageMapToColors.h"
#include "vtkImagePlaneWidget.h"
#include "vtkImageReader.h"
#include "vtkImageReslice.h"
#include "vtkInteractorEventRecorder.h"
#include "vtkInteractorStyleImage.h"
#include "vtkLookupTable.h"
#include "vtkNrrdReader.h"
#include "vtkOutlineFilter.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkResliceCursor.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursorWidget.h"
#include "vtkSmartPointer.h"
#include "vtkTextActor.h"
#include "vtkTransform.h"

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);

std::map<int, std::vector<double>> sliceNomarls
    = { { 0, { 1, 0, 0 } }, { 1, { 0, 1, 0 } }, { 2, { 0, 0, 1 } } };

class vtkResliceCursorCallback : public vtkCommand
{
public:
    static vtkResliceCursorCallback* New()
    {
        return new vtkResliceCursorCallback;
    }
    void Execute(vtkObject* caller, unsigned long event,
        void* callData) override
    {
        // vtkImagePlaneWidget* ipw = dynamic_cast<vtkImagePlaneWidget*>(caller);
        // if (ipw)
        // {
        //     double* wl = static_cast<double*>(callData);

        //     if (ipw == this->IPW[0])
        //     {
        //         this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
        //         this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
        //     }
        //     else if (ipw == this->IPW[1])
        //     {
        //         this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
        //         this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
        //     }
        //     else if (ipw == this->IPW[2])
        //     {
        //         this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
        //         this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
        //     }
        // }
        vtkResliceCursorWidget* rcw
            = dynamic_cast<vtkResliceCursorWidget*>(caller);
        if (rcw)
        {
            vtkResliceCursorLineRepresentation* rep
                = dynamic_cast<vtkResliceCursorLineRepresentation*>(
                    rcw->GetRepresentation());
            vtkResliceCursor* rc = rep->GetResliceCursorActor()
                                       ->GetCursorAlgorithm()
                                       ->GetResliceCursor();
            for (int i = 0; i < 3; i++)
            {
                vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(
                    this->IPW[i]->GetPolyDataAlgorithm());
                ps->SetNormal(sliceNomarls[i].data());
                std::cout << i << "normal: " << rc->GetPlane(i)->GetNormal()[0]
                          << " " << rc->GetPlane(i)->GetNormal()[1] << " "
                          << rc->GetPlane(i)->GetNormal()[2] << std::endl;
                ps->SetCenter(rc->GetPlane(i)->GetOrigin());
                std::cout << i << "origin: " << rc->GetPlane(i)->GetOrigin()[0]
                          << " " << rc->GetPlane(i)->GetOrigin()[1] << " "
                          << rc->GetPlane(i)->GetOrigin()[2] << std::endl;
                this->IPW[i]->UpdatePlacement();
            }
        }
        this->RCW[0]->Render();
    }
    vtkResliceCursorCallback() { }
    vtkImagePlaneWidget* IPW[3];
    vtkResliceCursorWidget* RCW[3];
};

int main()
{
    // vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    vtkSmartPointer<vtkNrrdReader> reader
        = vtkSmartPointer<vtkNrrdReader>::New();
    reader->SetFileName("D:/MRI.nrrd");
    reader->Update();

    /*vtkSmartPointer<vtkOutlineFilter> outline = vtkSmartPointer<vtkOutlineFilter>::New();
    outline->SetInputConnection(reader->GetOutputPort());*/

    vtkSmartPointer<vtkPolyDataMapper> outlineMapper
        = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(reader->GetOutputPort());

    vtkSmartPointer<vtkActor> outlineActor = vtkSmartPointer<vtkActor>::New();
    outlineActor->SetMapper(outlineMapper);

    vtkSmartPointer<vtkRenderer> ren[4];
    vtkSmartPointer<vtkRenderWindow> renWin
        = vtkSmartPointer<vtkRenderWindow>::New();
    renWin->SetMultiSamples(0);

    for (int i = 0; i < 4; i++)
    {
        ren[i] = vtkSmartPointer<vtkRenderer>::New();
        renWin->AddRenderer(ren[i]);
    }

    vtkSmartPointer<vtkRenderWindowInteractor> iren
        = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renWin);

    vtkSmartPointer<vtkCellPicker> picker
        = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);

    vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

    vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
    int imageDims[3];
    reader->GetOutput()->GetDimensions(imageDims);

    for (int i = 0; i < 3; i++)
    {
        planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        planeWidget[i]->SetInteractor(iren);
        planeWidget[i]->SetPicker(picker);
        planeWidget[i]->RestrictPlaneToVolumeOn();
        double color[3] = { 0, 0, 0 };
        color[i] = 1;
        planeWidget[i]->GetPlaneProperty()->SetColor(color);
        planeWidget[i]->SetTexturePlaneProperty(ipwProp);
        planeWidget[i]->TextureInterpolateOff();
        planeWidget[i]->SetResliceInterpolateToLinear();
        planeWidget[i]->SetInputConnection(reader->GetOutputPort());
        planeWidget[i]->SetPlaneOrientation(i);
        planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
        planeWidget[i]->DisplayTextOn();
        planeWidget[i]->SetDefaultRenderer(ren[3]);
        planeWidget[i]->SetWindowLevel(1358, -27);
        planeWidget[i]->On();
        planeWidget[i]->InteractionOn();
    }

    planeWidget[1]->SetLookupTable(planeWidget[0]->GetLookupTable());
    planeWidget[2]->SetLookupTable(planeWidget[0]->GetLookupTable());

    vtkSmartPointer<vtkResliceCursorCallback> cbk
        = vtkSmartPointer<vtkResliceCursorCallback>::New();
    vtkSmartPointer<vtkResliceCursor> resliceCursor
        = vtkSmartPointer<vtkResliceCursor>::New();
    resliceCursor->SetCenter(reader->GetOutput()->GetCenter());
    resliceCursor->SetThickMode(0);
    resliceCursor->SetThickness(10, 10, 10);
    resliceCursor->SetImage(reader->GetOutput());

    vtkSmartPointer<vtkResliceCursorWidget> resliceCursorWidget[3];
    vtkSmartPointer<vtkResliceCursorLineRepresentation> resliceCursorRep[3];

    double viewUp[3][3] = { { 0, 0, -1 }, { 0, 0, 1 }, { 0, 1, 0 } };
    for (int i = 0; i < 3; i++)
    {
        resliceCursorWidget[i] = vtkSmartPointer<vtkResliceCursorWidget>::New();
        resliceCursorWidget[i]->SetInteractor(iren);

        resliceCursorRep[i]
            = vtkSmartPointer<vtkResliceCursorLineRepresentation>::New();
        resliceCursorWidget[i]->SetRepresentation(resliceCursorRep[i]);
        resliceCursorRep[i]
            ->GetResliceCursorActor()
            ->GetCursorAlgorithm()
            ->SetResliceCursor(resliceCursor);
        resliceCursorRep[i]
            ->GetResliceCursorActor()
            ->GetCursorAlgorithm()
            ->SetReslicePlaneNormal(i);

        const double minVal = reader->GetOutput()->GetScalarRange()[0];
        if (vtkImageReslice* reslice
            = vtkImageReslice::SafeDownCast(resliceCursorRep[i]->GetReslice()))
        {
            reslice->SetBackgroundColor(minVal, minVal, minVal, minVal);
        }

        resliceCursorWidget[i]->SetDefaultRenderer(ren[i]);
        resliceCursorWidget[i]->SetEnabled(1);

        ren[i]->GetActiveCamera()->SetFocalPoint(0, 0, 0);
        double camPos[3] = { 0, 0, 0 };
        camPos[i] = 1;
        ren[i]->GetActiveCamera()->SetPosition(camPos);
        ren[i]->GetActiveCamera()->ParallelProjectionOn();
        ren[i]->GetActiveCamera()->SetViewUp(viewUp[i][0], viewUp[i][1],
            viewUp[i][2]);
        ren[i]->ResetCamera();
        cbk->IPW[i] = planeWidget[i];
        cbk->RCW[i] = resliceCursorWidget[i];
        resliceCursorWidget[i]->AddObserver(
            vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
        double range[2];
        reader->GetOutput()->GetScalarRange(range);
        resliceCursorRep[i]->SetWindowLevel(range[1] - range[0],
            (range[0] + range[1]) / 2.0);
        planeWidget[i]->SetWindowLevel(range[1] - range[0],
            (range[0] + range[1]) / 2.0);
        resliceCursorRep[i]->SetLookupTable(
            resliceCursorRep[0]->GetLookupTable());
        planeWidget[i]->GetColorMap()->SetLookupTable(
            resliceCursorRep[0]->GetLookupTable());
    }

    /*vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
    textActor->SetInput("横断面");
    textActor->GetProperty()->SetColor(0.0,1.0,0.0);*/

    ren[0]->SetBackground(0.3, 0.1, 0.1);
    ren[1]->SetBackground(0.1, 0.3, 0.1);
    ren[2]->SetBackground(0.1, 0.1, 0.3);
    ren[3]->AddActor(outlineActor);
    ren[3]->SetBackground(0.1, 0.1, 0.1);
    renWin->SetSize(600, 600);

    ren[0]->SetViewport(0, 0, 0.5, 0.5);
    ren[1]->SetViewport(0.5, 0, 1, 0.5);
    ren[2]->SetViewport(0, 0.5, 0.5, 1);
    ren[3]->SetViewport(0.5, 0.5, 1, 1);
    renWin->Render();

    ren[3]->GetActiveCamera()->Elevation(110);
    ren[3]->GetActiveCamera()->SetViewUp(0, 0, -1);
    ren[3]->GetActiveCamera()->Azimuth(45);
    ren[3]->GetActiveCamera()->Dolly(1.15);
    ren[3]->ResetCameraClippingRange();

    vtkSmartPointer<vtkInteractorStyleImage> style
        = vtkSmartPointer<vtkInteractorStyleImage>::New();
    iren->SetInteractorStyle(style);
    iren->Initialize();
    iren->Start();
    return EXIT_SUCCESS;
}
