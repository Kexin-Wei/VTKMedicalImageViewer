#include <vtkCallbackCommand.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageActor.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkInteractorStyleImage.h>
#include <vtkMatrix4x4.h>
#include <vtkNrrdReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkXMLImageDataReader.h>

#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);

// The mouse motion callback, to pick the image and recover pixel values.
class vtkImageInteractionCallback : public vtkCallbackCommand
{
public:
    static vtkImageInteractionCallback* New()
    {
        return new vtkImageInteractionCallback;
    }

    vtkImageInteractionCallback()
    {
        this->ImageReslicex = nullptr;
        this->ImageReslicey = nullptr;
        this->ImageReslicez = nullptr;
        this->Interactor = nullptr;
    }

    ~vtkImageInteractionCallback()
    {
        this->ImageReslicex = nullptr;
        this->ImageReslicey = nullptr;
        this->ImageReslicez = nullptr;
        this->Interactor = nullptr;
    }

    virtual void Execute(vtkObject*, unsigned long eventId,
        void* vtkNotUsed(callData)) override
    {
        switch (eventId)
        {
        case vtkCommand::LeftButtonPressEvent:
            this->Pick(eventId);
            break;
        case vtkCommand::MouseMoveEvent:
            this->MouseMove(eventId);
            break;
        case vtkCommand::LeftButtonReleaseEvent:
            this->Release(eventId);
            break;
        }
    }

    void SetInteractor(vtkRenderWindowInteractor* interactor)
    {
        this->Interactor = interactor;
    }

    vtkSmartPointer<vtkImageReslice> ImageReslicex;
    vtkSmartPointer<vtkImageReslice> ImageReslicey;
    vtkSmartPointer<vtkImageReslice> ImageReslicez;
    vtkSmartPointer<vtkRenderWindowInteractor> Interactor;

private:
    void Pick(unsigned long eventId)
    {
        std::cout << vtkCommand::GetStringFromEventId(eventId) << std::endl;
        int X = this->Interactor->GetEventPosition()[0];
        int Y = this->Interactor->GetEventPosition()[1];
        std::cout << "X: " << X << " Y: " << Y << std::endl;
    }

    void MouseMove(unsigned long eventId)
    {
        std::cout << vtkCommand::GetStringFromEventId(eventId) << std::endl;
        int X = this->Interactor->GetEventPosition()[0];
        int Y = this->Interactor->GetEventPosition()[1];
        std::cout << "X: " << X << " Y: " << Y << std::endl;
    }

    void Release(unsigned long eventId)
    {
        std::cout << vtkCommand::GetStringFromEventId(eventId) << std::endl;
        int X = this->Interactor->GetEventPosition()[0];
        int Y = this->Interactor->GetEventPosition()[1];
        std::cout << "X: " << X << " Y: " << Y << std::endl;
    }
};

void initImageActor(double* Matrix, double* center, double* spacing,
    vtkSmartPointer<vtkImageCast> pImageCast,
    vtkSmartPointer<vtkImageReslice> imageReslice,
    vtkSmartPointer<vtkImageActor> actor)
{
    vtkMatrix4x4* AxialResliceMatrix = vtkMatrix4x4::New();
    AxialResliceMatrix->DeepCopy(Matrix);

    AxialResliceMatrix->SetElement(0, 3, center[0]);
    AxialResliceMatrix->SetElement(1, 3, center[1]);
    AxialResliceMatrix->SetElement(2, 3, center[2]);

    // 设置体数据来源
    imageReslice->SetInputConnection(pImageCast->GetOutputPort());
    // 设置输出是一个切片，而不是一个卷
    imageReslice->SetOutputDimensionality(2);
    imageReslice->SetOutputSpacing(spacing);
    // pImageResliceX->SetResliceAxesDirectionCosines(sagittalX, sagittalY, sagittalZ);
    // 设置vtkImageReslice的切面坐标系矩阵
    imageReslice->SetResliceAxes(AxialResliceMatrix);
    // pImageResliceX->SetResliceAxesOrigin(center);
    // 设置切面算法的插值方式为线性插值
    imageReslice->SetInterpolationModeToLinear();
    imageReslice->Update();
    // pImageMapToColorsX->SetLookupTable(pWindowLevelLookupTable);
    // pImageMapToColorsX->SetInputConnection(pImageResliceX->GetOutputPort());
    imageReslice->SetInterpolationModeToLinear();
    actor->SetInputData(imageReslice->GetOutput());
    actor->SetPosition(0, 0, 0);
    actor->Update();
};

int main()
{
    vtkSmartPointer<vtkImageReslice> pImageResliceX = vtkSmartPointer<vtkImageReslice>::New();
    vtkSmartPointer<vtkImageReslice> pImageResliceY = vtkSmartPointer<vtkImageReslice>::New();
    vtkSmartPointer<vtkImageReslice> pImageResliceZ = vtkSmartPointer<vtkImageReslice>::New();

    vtkSmartPointer<vtkXMLImageDataReader> pXMLImageDataReader = vtkSmartPointer<vtkXMLImageDataReader>::New();
    vtkSmartPointer<vtkImageCast> pImageCast = vtkSmartPointer<vtkImageCast>::New();

    vtkSmartPointer<vtkImageActor> pImageActorX = vtkSmartPointer<vtkImageActor>::New();
    vtkSmartPointer<vtkImageActor> pImageActorY = vtkSmartPointer<vtkImageActor>::New();
    vtkSmartPointer<vtkImageActor> pImageActorZ = vtkSmartPointer<vtkImageActor>::New();

    vtkSmartPointer<vtkRenderer> pRendererX = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> pRendererY = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> pRendererZ = vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderer> pRenderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> pRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();

    // vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
    // reader->SetFileName("D:/datasource/brain.mhd");
    // reader->Update();
    // vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    vtkSmartPointer<vtkNrrdReader> reader
        = vtkSmartPointer<vtkNrrdReader>::New();
    reader->SetFileName("D:/MRI.nrrd");
    reader->Update();

    int extent[6];
    double spacing[3];
    double origin[3];

    reader->GetOutput()->GetExtent(extent);
    reader->GetOutput()->GetSpacing(spacing);
    reader->GetOutput()->GetOrigin(origin);

    // 计算中心位置。
    double center[3];
    center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
    center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
    center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);

    // 轴状面
    double Axial[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    // 冠状面
    double Coronal[16] = {
        1, 0, 0, 0,
        0, 0, -1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1
    };
    // 矢状面
    double Sagittal[16] = {
        0, 0, 1, 0,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 1
    };

    pImageCast->SetInputConnection(reader->GetOutputPort());
    pImageCast->SetOutputScalarTypeToChar();
    pImageCast->ClampOverflowOn();
    pImageCast->Update();
    // pImageCast->SetUpdateExtentToWholeExtent();

    // x
    initImageActor(Axial, center, spacing, pImageCast, pImageResliceX,
        pImageActorX);

    // y
    initImageActor(Coronal, center, spacing, pImageCast, pImageResliceY,
        pImageActorY);

    // z
    initImageActor(Sagittal, center, spacing, pImageCast, pImageResliceZ,
        pImageActorZ);

    float fOpac = 0.5;

    pRendererX->AddActor(pImageActorX);
    pRendererY->AddActor(pImageActorY);
    pRendererZ->AddActor(pImageActorZ);

    // pRenderer->SetBackground(1, 1, 1);
    pRendererX->SetBackground(0, 0, 0);
    pRendererY->SetBackground(0, 0, 0);
    pRendererZ->SetBackground(0, 0, 0);

    /*
        #  renderer 0: BOTTOM LEFT
        #  renderer 1: BOTTOM RIGHT
        #  renderer 2: TOP LEFT
        #  renderer 3: TOP RIGHT
    */
    double ltView[4] = { 0, 0, 0.5, 0.5 };
    double rtView[4] = { 0.5, 0, 1, 0.5 };
    double lbView[4] = { 0, 0.5, 0.5, 1 };
    double rbView[4] = { 0.5, 0.5, 1, 1 };

    // pRenderer->SetViewport(0, 0, 0.6, 1);
    // pRendererX->SetViewport(0.6, 0.66, 1, 1);
    // pRendererY->SetViewport(0.6, 0.33, 1, 0.66);
    // pRendererZ->SetViewport(0.6, 0, 1, 0.33);

    pRenderer->SetViewport(rtView);
    pRendererX->SetViewport(lbView);
    pRendererY->SetViewport(rbView);
    pRendererZ->SetViewport(ltView);

    pRenderWindow->AddRenderer(pRendererX);
    pRenderWindow->AddRenderer(pRendererY);
    pRenderWindow->AddRenderer(pRendererZ);
    pRenderWindow->AddRenderer(pRenderer);

    vtkSmartPointer<vtkRenderWindowInteractor> pRenderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    pRenderWindow->SetSize(600, 600);

    // add observer;
    vtkSmartPointer<vtkInteractorStyleImage> imagestyle = vtkSmartPointer<vtkInteractorStyleImage>::New();

    pRenderWindowInteractor->SetInteractorStyle(imagestyle);
    pRenderWindowInteractor->SetRenderWindow(pRenderWindow);
    pRenderWindowInteractor->Initialize();

    vtkSmartPointer<vtkImageInteractionCallback> callback = vtkSmartPointer<vtkImageInteractionCallback>::New();
    // callback->SetImageReslice(reslice);
    callback->ImageReslicex = pImageResliceX;
    callback->ImageReslicey = pImageResliceY;
    callback->ImageReslicez = pImageResliceZ;
    callback->SetInteractor(pRenderWindowInteractor);
    // callback->SetImageMapToColors(colorMap);

    imagestyle->AddObserver(vtkCommand::MouseMoveEvent, callback);
    imagestyle->AddObserver(vtkCommand::LeftButtonPressEvent, callback);
    imagestyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback);

    pRenderWindow->Render();

    pRenderWindowInteractor->Initialize();
    pRenderWindowInteractor->Start();

    return 0;
}