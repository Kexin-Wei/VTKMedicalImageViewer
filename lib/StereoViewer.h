#pragma once

#include "BaseViewer.h"
#include "unit.h"
#include <QFrame>
#include <QVBoxLayout>
#include <QVTKOpenGLWidget.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

class vtkImageStack;

namespace ultrast {
namespace infrastructure {
namespace utility {
class PointSet;
}
}
}

using PointSet = ultrast::infrastructure::utility::PointSet;
namespace unit = ultrast::infrastructure::utility::unit;

namespace ultrast {
namespace visualization {

class StereoViewer;

class StereoViewerInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static StereoViewerInteractorStyle* New();
    void SetRenderer(vtkRenderer* renderer);
    void SetRenderWindow(vtkRenderWindow* renderWindow);
    void SetStereoViewer(StereoViewer* stereoViewer);
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;
    void zoom(const bool& isIn, double scale = 0);

protected:
    StereoViewerInteractorStyle();

private:
    void dollyToPosition(double factor, double* position);
    vtkRenderer* m_renderer;
    vtkRenderWindow* m_renderWindow;
    StereoViewer* m_stereoViewer;
    double m_scaleFactor;
};

class StereoViewer : public BaseViewer
{
    Q_OBJECT
public:
    StereoViewer(QWidget* parent);
    virtual ~StereoViewer();
    const QImage capture() override;
    void update() override;

    vtkSmartPointer<vtkRenderer> getRenderer() const { return m_renderer; }

    void addVolumeImage(VolumeImage& volumeImage) override;
    void removeVolumeImage(VolumeImage& volumeImage) override;
    // void addModel(Model& model) override;
    // void removeModel(Model& model) override;
    // void addMarker(MarkerPoint& marker) override;
    // void removeMarker(MarkerPoint& marker) override;
    // void addPointSet(PointSet& pointset) override;
    // void removePointSet(PointSet& pointset) override;
    // void addImage2d(Image2d& image2d) override;
    // void removeImage2d(Image2d& image2d) override;
    void bindViewerManager(ViewerManager* viewerManager) override;
    double getViewAngle();
    void setViewAngle(double angle);

    unit::Point getCurrentCoord() const { return m_currentCoord; };

    vtkCamera* getCamera();
    void setCamera(vtkCamera* camera);

signals:
    void coordinateChanged(const unit::Point& coord);
    void zoomChanged();
public slots:
    void render() override;
    // void resetCamera() override;
    void resetCamera(double bounds[6]);
    void storeCoordinate(const unit::Point& coord);
    void zoomIn(double scale = 0);
    void zoomOut(double scale = 0);

private:
    void initGui();
    void initInteratorStyle();
    void initCamera();
    void initImageStacks();

    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> m_it;
    vtkSmartPointer<StereoViewerInteractorStyle> m_interactorStyle;

    QVTKOpenGLWidget* m_openGLWidget;
    std::vector<vtkSmartPointer<vtkImageStack>> m_imageStacks;
    // stereo viewer has no info on current coord
    // so it is stored each time it gets passed it instead
    unit::Point m_currentCoord;
    bool startMeasure;
    double m_scaleFactor;
};

} // namespace visualization
}; // namespace ultrast