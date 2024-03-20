#pragma once

#include "SliceViewer.h"
#include "unit.h"
#include <QWidget>

class QSplitter;

// namespace ultrast {
// namespace domain {
// namespace data {
// class DataManager;
// }
// }
// }

namespace ultrast {
namespace infrastructure {
namespace utility {
enum VisualAxis : unsigned int;
class Data3d;
class VolumeImage;
}
}
}

using Data3d = ultrast::infrastructure::utility::Data3d;
// using DataManager = ultrast::domain::data::DataManager;
using VisualAxis = ultrast::infrastructure::utility::VisualAxis;
using VolumeImage = ultrast::infrastructure::utility::VolumeImage;
namespace unit = ultrast::infrastructure::utility::unit;

namespace ultrast {
namespace visualization {

class SliceViewer;
class StereoViewer;
class ViewerManager;
class BaseViewer;
class FlexibleSliceViewer;
// class Volume3DMeasurementWidget;

class QuadViewerWidget : public QWidget
{
    Q_OBJECT
public:
    enum ViewerLayout : unsigned int
    {
        DEFAULT,
        BIOPSY_PLAN,
        BIOPSY_OPERATION,
        REGISTRATION
    };

    QuadViewerWidget(QWidget* parent = nullptr,
        const QString& coordinateId = QString());
    ~QuadViewerWidget();
    void resetCamera();
    void resetCamera(double bounds[6]);
    void createSlicerViewerWidgets();
    void deleteSlicerViewerWidgets();
    // void listenForDistanceMeasurement(bool enable);
    // void listenForAngleMeasurement(bool enable);
    // void listenForVolumeMeasurement(bool enable);
    void render();
    const unit::Point getCrosshairCoordinate();
    void setStereoViewerVisible(const bool& isVisible);
    // void filterOutVolumeImage(const std::vector<VolumeImage*>& images);
    // void resetVolumeImageFilters();
    void addData(Data3d& data);
    void removeData(Data3d& data);
    bool isStereoViewerVisible();
    bool hasStereoViewer();
    // void resetMaxSizedViewer();
    // Volume3DMeasurementWidget* get3dVolumeMeasurer()
    // {
    //     return m_3dVolumeMeasurer;
    // };
    SliceViewer* getShowTextActorViewer() { return m_axialViewer; };
    SliceViewer* getViewerByLable(QString label);
    void registerExternalStereoViewer(StereoViewer* stereoViewer);
    // void setFlexibleSliceViewerVisible(const bool& isVisible);
    // void setImageHorizontalFlip(const bool& horizontalFlip);
    // void setImageVerticalFlip(const bool& verticalFlip);
    void resetCameraAndSetCrosshair(double bounds[6], const unit::Point& point);
    vtkCamera* getAxialViewerCamera();
    vtkCamera* getCoronalViewerCamera();
    vtkCamera* getSagittalViewerCamera();
    vtkCamera* getStereoViewerCamera();
    void setAxialViewerCamera(vtkCamera* camera);
    void setCoronalViewerCamera(vtkCamera* camera);
    void setSagittalViewerCamera(vtkCamera* camera);
    void setStereoViewerCamera(vtkCamera* camera);
    QImage captureQuadViewerWidget();
    QImage captureIndividualViewer(QWidget* viewer);
    void setAllDataBounds(double* bounds);
    void setFocalPoint(unit::Point focalPoint);

signals:
    void resizeWidget(QWidget* widget, const bool& isMaxsized);
    void buttonModifyOrigin(VisualAxis axis, bool increase);
    void buttonModifyOrientation(VisualAxis axis, bool increase);
    void buttonModifySize(VisualAxis axis, bool increase);
    void leftButtonDown(const unit::Point& coord);
    void middleButtonDown(const unit::Point& coord);
    void middleButtonUp(const unit::Point& coord);
    void rightButtonDown(const unit::Point& coord);
    void coordinateChanged(const unit::Point& coord);
    void robotPoseUpdate(unit::Pose pose);
    void sliceViewerZoomChanged(const double zoomFactor);
    void stereoViewerZoomChanged(const bool isZoomIn);
    void stereoViewerVisible(bool visible);
    void stereoViewerResizeClicked();

public slots:
    void setCrosshairCoordinate(const unit::Point& coord);
    void updateAllDataBounds();
    void zoomIn();
    void zoomOut();
    void zoomSliceViewer(const double zoomFactor);
    void zoomStereoViewer(const bool isZoomIn);
    void stereoZoomIn();
    void stereoZoomOut();
    void setViewerLayout(const ViewerLayout& layout);
    void updateScreenOrientation(const Qt::Orientation orientation);

    void leftClick(QString viewerLabel, const unit::Point& coord);
    void mouseMove(QString viewerLabel, const unit::Point& coord);
    // void deleteVolumeWidgets(bool isAll, MeasurementType measurementType,
    //     int subIndex);
    // void reviewMaximizeStereoViewer(const bool& toBeMaximized);
private slots:
    void resizeChild(BaseViewer* viewer);

private:
    void initializeUI();
    void bindConnections();
    // void setAllDataBoundsToDefault();
    void setAllDataBoundsToSliceViewers();
    void setSplitterSize(QSplitter& splitter, const int maxSizeValue);

    SliceViewer* m_axialViewer;
    SliceViewer* m_coronalViewer;
    SliceViewer* m_sagittalViewer;
    StereoViewer* m_stereoViewer;
    bool m_stereoViewerOldVisibility;
    double m_stereoViewerOldViewAngle;
    // FlexibleSliceViewer* m_flexibleSliceViewer;
    QSplitter* m_mainSplitter;
    QSplitter* m_topSplitter;
    QSplitter* m_middleSplitter;
    QSplitter* m_bottomSplitter;
    ViewerManager* m_viewerManager;
    // DataManager* m_dataManager;
    // in order of xMin, xMax, yMin, yMax, zMin, zMax
    double m_allDataBounds[6];
    QVector<SliceViewer*> m_sliceViewers;
    QVector<StereoViewer*> m_stereoViewers;
    QSet<BaseViewer*> m_activeViewers;
    Qt::Orientation m_screenOrientation;
    // Volume3DMeasurementWidget* m_3dVolumeMeasurer;
};
}
}
