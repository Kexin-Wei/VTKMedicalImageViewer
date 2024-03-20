#include "ViewerManager.h"
#include <QDebug>
#include <QTimer>
#include <QVTKOpenGLWidget.h>
// #include <domain\data\VolumeImageManager.h>
#include <Constants.h>
// #include <Logger.h>
// #include <MarkerPoint.h>
// #include <Model.h>
// #include <PointSet.h>
#include <StereoViewer.h>
#include <VolumeImage.h>
#include <vtkImageStack.h>

// using Model = ultrast::infrastructure::utility::Model;
using VolumeImage = ultrast::infrastructure::utility::VolumeImage;
// using Logger = ultrast::infrastructure::utility::Logger;
// using VolumeImageManager = ultrast::domain::data::VolumeImageManager;

namespace ultrast {
namespace visualization {

StereoViewer::StereoViewer(QWidget* parent) :
    BaseViewer(parent, "Stereo Viewer"),
    m_scaleFactor(1.1),
    m_renderer(vtkSmartPointer<vtkRenderer>::New()),
    m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New())
{
    m_type = ViewerType::STEREO;
    initGui();
    initInteratorStyle();
    initCamera();
    initImageStacks();
    // addVtkResizeButton(m_renderer, m_it);
}

StereoViewer::~StereoViewer()
{
}

const QImage StereoViewer::capture()
{
    this->render();
    return m_openGLWidget->grabFramebuffer();
}

void StereoViewer::update()
{
}

void StereoViewer::addVolumeImage(VolumeImage& volumeImage)
{
    qDebug() << "StereoViewer - adding volume image";
    for (int i = 0; i < m_imageStacks.size(); ++i)
        m_imageStacks[i]->AddImage(
            volumeImage.getStereoImageSlice(getViewerId())[i]);
    m_renderer->AddActor(volumeImage.getAssembly(getViewerId()));
    connect(this, &StereoViewer::coordinateChanged, &volumeImage,
        &VolumeImage::setStereoSlicePosition, Qt::DirectConnection);
}

void StereoViewer::removeVolumeImage(VolumeImage& volumeImage)
{
    for (int i = 0; i < m_imageStacks.size(); ++i)
        m_imageStacks[i]->RemoveImage(
            volumeImage.getStereoImageSlice(getViewerId())[i]);
    m_renderer->RemoveActor(volumeImage.getAssembly(getViewerId()));
}

// void StereoViewer::addModel(Model& model)
// {
//     m_renderer->AddActor(model.getAssembly(getViewerId()));
// }

// void StereoViewer::removeModel(Model& model)
// {
//     m_renderer->RemoveActor(model.getAssembly(getViewerId()));
// }

// void StereoViewer::addMarker(MarkerPoint& marker)
// {
//     m_renderer->AddActor(marker.getActor(getViewerId()));
// }

// void StereoViewer::removeMarker(MarkerPoint& marker)
// {
//     m_renderer->RemoveActor(marker.getActor(getViewerId()));
// }

// void StereoViewer::addPointSet(PointSet& pointset)
// {
//     m_renderer->AddActor(pointset.getActor(getViewerId()));
// }

// void StereoViewer::removePointSet(PointSet& pointset)
// {
//     m_renderer->RemoveActor(pointset.getActor(getViewerId()));
// }

// void StereoViewer::addImage2d(Image2d& image2d)
// {
// }

// void StereoViewer::removeImage2d(Image2d& image2d)
// {
// }

vtkCamera* StereoViewer::getCamera()
{
    return m_renderer->GetActiveCamera();
}

void StereoViewer::setCamera(vtkCamera* camera)
{
    if (camera == nullptr)
    {
        return;
    }

    m_renderer->SetActiveCamera(camera);
}

void StereoViewer::bindViewerManager(ViewerManager* viewerManager)
{
    connect(viewerManager, &ViewerManager::coordinateChanged, this,
        &StereoViewer::storeCoordinate);
}

void StereoViewer::zoomIn(double scale)
{
    m_interactorStyle->zoom(true, scale);
}

void StereoViewer::zoomOut(double scale)
{
    m_interactorStyle->zoom(false, scale);
}

void StereoViewer::initGui()
{
    m_openGLWidget = new QVTKOpenGLWidget;
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_openGLWidget);

    m_renderWindow->AddRenderer(m_renderer);
    m_openGLWidget->SetRenderWindow(m_renderWindow);

    vtkNew<vtkNamedColors> colors;
    std::array<unsigned char, 4> bkg { { 0, 0, 0, 255 } };
    colors->SetColor("BkgColor", bkg.data());
    m_renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
}

void StereoViewer::initInteratorStyle()
{
    m_it = m_renderWindow->GetInteractor();
    m_interactorStyle = vtkSmartPointer<StereoViewerInteractorStyle>::New();
    m_interactorStyle->SetRenderer(m_renderer);
    m_interactorStyle->SetRenderWindow(m_renderWindow);
    m_interactorStyle->SetStereoViewer(this);
    m_it->SetInteractorStyle(m_interactorStyle);
}

void StereoViewer::initCamera()
{
    vtkNew<vtkCamera> aCamera;
    aCamera->SetViewUp(0, -1, 0);
    aCamera->SetPosition(0, 0, -1);
    m_renderer->SetActiveCamera(aCamera);
}

void StereoViewer::initImageStacks()
{
    for (int i = 0; i < 3; ++i)
    {
        auto stack = vtkSmartPointer<vtkImageStack>::New();
        m_imageStacks.push_back(stack);
        m_renderer->AddActor(stack);
    }
}

void StereoViewer::render()
{
    if (isVisible())
    { // no need to render if widget is hidden
        m_renderer->ResetCameraClippingRange();
        m_renderWindow->Render();
    }
}

double StereoViewer::getViewAngle()
{
    return m_renderer->GetActiveCamera()->GetViewAngle();
}

void StereoViewer::setViewAngle(double angle)
{
    m_renderer->GetActiveCamera()->SetViewAngle(angle);
}

// void StereoViewer::resetCamera()
// {
//     m_renderer->GetActiveCamera()->SetViewUp(0, -1, 0);
//     m_renderer->GetActiveCamera()->SetPosition(0, 0, -800);
//     std::vector<double> focals
//         = VolumeImageManager::getInstance()->getResetCameraFocalPoint();
//     m_renderer->GetActiveCamera()->SetFocalPoint(focals[0], focals[1],
//         focals[2]);
//     m_renderer->GetActiveCamera()->SetViewAngle(
//         STEREOVIEWER_DEFAULT_VIEW_ANGLE);
//     m_renderer->GetActiveCamera()->Azimuth(-15);
//     render();
// }

void StereoViewer::resetCamera(double bounds[6])
{
    // resetCamera();
}

void StereoViewer::storeCoordinate(const unit::Point& coord)
{
    m_currentCoord = coord;
    emit coordinateChanged(m_currentCoord);
    render();
}

StereoViewerInteractorStyle* StereoViewerInteractorStyle::New()
{
    return new StereoViewerInteractorStyle();
}

void StereoViewerInteractorStyle::SetRenderer(vtkRenderer* renderer)
{
    if (renderer == nullptr)
    {
        return;
    }

    m_renderer = renderer;
}

void StereoViewerInteractorStyle::SetRenderWindow(vtkRenderWindow* renderWindow)
{
    if (renderWindow == nullptr)
    {
        return;
    }

    m_renderWindow = renderWindow;
}

void StereoViewerInteractorStyle::SetStereoViewer(StereoViewer* stereoViewer)
{
    if (stereoViewer == nullptr)
    {
        return;
    }

    m_stereoViewer = stereoViewer;
}

void StereoViewerInteractorStyle::OnMouseWheelForward()
{
    zoom(true);
}

void StereoViewerInteractorStyle::OnMouseWheelBackward()
{
    zoom(false);
}

void StereoViewerInteractorStyle::zoom(const bool& isIn, double scale)
{
    const unit::Point crosshairPoint = m_stereoViewer->getCurrentCoord();
    double point[3]
        = { crosshairPoint[0], crosshairPoint[1], crosshairPoint[2] };
    double crosshairPosDisplay[3] = { 0 };

    auto camera = m_renderer->GetActiveCamera();
    auto pos = camera->GetPosition();
    auto fp = camera->GetFocalPoint();
    double worldDistance[3] = { 0 };
    vtkMath::Subtract(point, fp, worldDistance);

    double translate[3]
        = { worldDistance[0], worldDistance[1], worldDistance[2] };

    if (scale == 0)
        scale = m_scaleFactor;
    double factor = isIn ? 1 - (1 / scale) : -(scale - 1);
    vtkMath::MultiplyScalar(translate, factor);

    double newFp[3] = { 0 };
    vtkMath::Add(fp, translate, newFp);

    camera->SetFocalPoint(newFp);

    double newPos[3] = { 0 };
    vtkMath::Add(pos, translate, newPos);
    camera->SetPosition(newPos);

    double zoomFactor = isIn ? scale : 1 / scale;
    camera->Zoom(zoomFactor);

    m_stereoViewer->render();

    emit m_stereoViewer->zoomChanged();
}

StereoViewerInteractorStyle::StereoViewerInteractorStyle() :
    m_renderer(nullptr),
    m_renderWindow(nullptr),
    m_stereoViewer(nullptr),
    m_scaleFactor(1.1)
{
}

} // namespace visualization
}; // namespace ultrast