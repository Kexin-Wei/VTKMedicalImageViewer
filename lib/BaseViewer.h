#pragma once

#include "VisualizationDataType.h"

#include <QFrame>
#include <QIcon>
#include <QPushButton>
#include <QResizeEvent>
#include <QSharedPointer>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

namespace ultrast {
namespace infrastructure {
namespace utility {
class Model;
class MarkerPoint;
class PointSet;
class VolumeImage;
class Data3d;
class Image2d;
}
}
}

using Data3d = ultrast::infrastructure::utility::Data3d;
using VolumeImage = ultrast::infrastructure::utility::VolumeImage;
using Image2d = ultrast::infrastructure::utility::Image2d;
using PointSet = ultrast::infrastructure::utility::PointSet;
using Model = ultrast::infrastructure::utility::Model;
using MarkerPoint = ultrast::infrastructure::utility::MarkerPoint;
using ViewerType = ultrast::infrastructure::utility::ViewerType;
using ViewerInfo = ultrast::infrastructure::utility::ViewerInfo;

namespace ultrast {
namespace visualization {

class VtkOverlayButton;
class QOverlayButton;
class ViewerManager;

class BaseViewer : public QFrame
{
    Q_OBJECT
public:
    BaseViewer(QWidget* parent, QString name = "");
    virtual ~BaseViewer();
    const ViewerType getType() const { return m_type; }
    const QString getViewerId() const { return m_viewerId; }
    virtual ViewerInfo getViewerInfo() const;
    virtual const QImage capture() = 0;
    virtual void update() = 0;
    virtual void addData(Data3d& data);
    virtual void removeData(Data3d& data);
    virtual void addTextActor(Data3d& data);
    virtual void removeTextActor(Data3d& data);
    // void addVtkResizeButton(vtkRenderer* renderer,
    //     vtkRenderWindowInteractor* it);
    // void setVtkResizeButtonEnable(vtkRenderer* renderer, bool visible);
    void addQResizeButton(QWidget* parent);
    void resizeEvent(QResizeEvent* event);
    virtual void addVolumeImage(VolumeImage& volumeImage) {};
    virtual void removeVolumeImage(VolumeImage& volumeImage) {};
    // virtual void addModel(Model& model) {};
    // virtual void addMarker(MarkerPoint& marker) {};
    // virtual void removeModel(Model& model) {};
    // virtual void removeMarker(MarkerPoint& marker) {};
    // virtual void addPointSet(PointSet& pointSet) {};
    // virtual void removePointSet(PointSet& pointSet) {};
    // virtual void addImage2d(Image2d& image2d) {};
    // virtual void removeImage2d(Image2d& image2d) {};
    virtual void bindViewerManager(ViewerManager* viewerManager) = 0;
    bool IsMaximized();
signals:
    void resizeSplitter(BaseViewer* viewer);
    void onFrameResize();
    void resizeButtonClicked();

public slots:
    // virtual void resizeSelf();
    virtual void onResizeButtonClicked();
    virtual void render() {};
    // virtual void resetCamera() {};
    virtual void resetCamera(double bounds[6]) {};

protected: //variables
    vtkSmartPointer<VtkOverlayButton> m_vtkResizeButton;
    QOverlayButton* m_qResizeButton;
    ViewerType m_type = ViewerType::INVALID;
    bool isMaximized = false;

private:
    static int m_idCounter;
    const QString m_viewerId;
};

} // namespace visualization
}; // namespace ultrast