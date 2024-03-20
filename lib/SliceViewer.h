#pragma once

#include "BaseViewer.h"
#include "vtkAutoInit.h"
#include <QFrame>
#include <QMap>
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkInteractionStyle)
#include "QVTKOpenGLWidget.h"
#include "VisualizationDataType.h"
#include "vtkActor.h"
#include "vtkAngleRepresentation.h"
#include "vtkAngleWidget.h"
#include "vtkAxesActor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkDistanceRepresentation.h"
#include "vtkDistanceWidget.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkImageData.h"
#include "vtkImageProperty.h"
#include "vtkImageResliceMapper.h"
#include "vtkImageSlice.h"
#include "vtkImageStack.h"
#include "vtkInteractorStyleImage.h"
#include "vtkLineRepresentation.h"
#include "vtkLineWidget2.h"
#include "vtkLookupTable.h"
#include "vtkParametricEllipsoid.h"
#include "vtkParametricFunctionSource.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkTextProperty.h"
// #include <BaseOverlayButton.h>
#include "unit.h"
#include <vtkBillboardTextActor3D.h>

namespace ultrast {
namespace infrastructure {
namespace utility {
class PointSet;
}
}
}

class QVTKOpenGLWidget;

namespace ultrast {
namespace visualization {

using MeasurementType = ultrast::infrastructure::utility::MeasurementType;
using VisualAxis = ultrast::infrastructure::utility::VisualAxis;
using PointSet = ultrast::infrastructure::utility::PointSet;
namespace unit = ultrast::infrastructure::utility::unit;

class SliceViewer;
class SliceViewerInteractorStyle : public vtkInteractorStyleImage
{
public:
    static SliceViewerInteractorStyle* New();
    void SetRenderer(vtkRenderer* renderer);
    void SetRenderWindow(vtkRenderWindow* renderWindow);
    void SetSliceViewer(SliceViewer* sliceViewer);
    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMiddleButtonDown() override;
    void OnMiddleButtonUp() override;
    void OnRightButtonDown() override;
    void OnRightButtonUp() override;
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;
    void zoom(const bool& isIn);

protected:
    SliceViewerInteractorStyle();

private:
    vtkRenderer* m_renderer;
    vtkRenderWindow* m_renderWindow;
    SliceViewer* m_sliceViewer;
    double m_scaleFactor;
};

class VtkCustomizedButton;
class SliceViewer : public BaseViewer
{
    Q_OBJECT
public:
    enum Orientation
    {
        LEFT,
        RIGHT,
        SUPERIOR,
        INFERIOR,
        ANTERIOR,
        POSTERIOR
    };
    SliceViewer(QWidget* parent, Orientation orientation);
    virtual ~SliceViewer();
    ViewerInfo getViewerInfo() const override;
    const QImage capture() override;
    void update() override;
    void bindViewerManager(ViewerManager* viewerManager) override;
    vtkSmartPointer<vtkRenderer> getRenderer() const { return m_renderer; }
    void addVolumeImage(VolumeImage& volumeImage) override;
    void removeVolumeImage(VolumeImage& volumeImage) override;
    // void addModel(Model& model) override;
    // void addMarker(MarkerPoint& marker) override;
    // void removeMarker(MarkerPoint& marker) override;
    // void removeModel(Model& model) override;
    // void addPointSet(PointSet& pointset) override;
    // void removePointSet(PointSet& pointset) override;
    // void addImage2d(Image2d& image2d) override;
    // void removeImage2d(Image2d& image2d) override;
    const unit::Point getCrosshairCoordinate() const;
    void centerCrosshair();
    void setAllDataBounds(double* bounds);
    const VisualAxis getThirdAxis();
    void setParallelScale(const double& parallelScale);
    /**
    * @brief Gets the event coordinates from the interactor and sets it to system display
    *
    */
    const unit::Point getEventCoord() const;
    vtkCamera* getCamera();
    void setCamera(vtkCamera* camera);
    VisualAxis getHorizontalAxis();
    VisualAxis getVerticalAxis();
    VisualAxis getNormalAxis();
    QString getLabel() const;
    vtkRenderWindowInteractor* getInteractor() { return m_it.Get(); }
    vtkRenderer* getRenderer() { return m_renderer.Get(); }
    QString getOrientationAsQString() const;

signals:
    //only from user interaction with viewer by mouse
    void coordinateChangedByMouse(const unit::Point& coord);
    //crosshair coordinate changed by mouse or by methods
    void coordinateChanged(const unit::Point& coord);
    //called with coordinate changed, is used to only update data that need to be sliced
    void updateSlicedDataCoordinate(const unit::Point& coord);
    void measurementStarted();
    void storeDistanceWidget(vtkSmartPointer<vtkDistanceWidget> distanceWidget,
        VisualAxis axis, double axisValue);
    void storeAngleWidget(vtkSmartPointer<vtkAngleWidget> angleWidget,
        VisualAxis axis, double axisValue);
    void leftButtonDown(const unit::Point& coord);
    void leftButtonUp(const unit::Point& coord);
    void middleButtonDown(const unit::Point& coord);
    void middleButtonUp(const unit::Point& coord);
    void rightButtonDown(const unit::Point& coord);
    void rightButtonUp(const unit::Point& coord);
    void mouseMove(QString viewerLabel, const unit::Point& coord);
    void leftClick(QString viewerLabel, const unit::Point& coord);
    //viewerType is difined as SliceViewer is being created, which is SUPPERIOR, LEFT, POSTERIOR (Orientation)
    void buttonModifyOrigin(VisualAxis axis, bool increase);
    void buttonModifyOrientation(VisualAxis axis, bool increase);
    void buttonModifySize(VisualAxis axis, bool increase);
    void zoomChanged();
public slots:
    void render() override;
    /** Resets camera to default crosshair position around defined motion space
    *
    * crosshair is set to the default defined values and camera is moved to motion space area
    **/
    // void resetCamera() override;
    void resetCamera(double bounds[6]) override;
    /** Centers camera to center of all data bounds
    *
    * crosshair is set to the center coord of all data bounds
    **/
    void centerCamera();
    void setCrosshairCoordinate(const unit::Point& coord, bool relay = false);
    /** Refreshes the length of the crosshair on all 3 axes
     * 
     * Length is based on m_allDataBounds.
     * 
     */
    void refreshCrosshairSize();
    //sets the focal point of camera in third axis and crosshair position to the 2 other axes
    void setCameraFocalPoint(const unit::Point& coord);
    // void disableMeasurement(int measurementType);
    // void listenDistanceMeasurement(bool enable);
    // void listenAngleMeasurement(bool enable);
    // void listenVolumeMeasurement(bool enable);
    void createSlicerViewerWidgets();
    void deleteSlicerViewerWidgets();
    void zoomIn();
    void zoomOut();
    void leftClickDown();
    void leftClickUp();
    void leftDrag();
    void rightClickDown();
    void rightClickUp();
    void middleClickDown();
    void middleClickUp();
private slots:
    void onButtonPositionUp();
    void onButtonPositionDown();
    void onButtonPositionLeft();
    void onButtonPositionRight();
    void onButtonRotationAntiClock();
    void onButtonRotationClock();
    void onButtonSizeVerticalIncrease();
    void onButtonSizeVerticalDecrease();
    void onButtonSizeHorizontalIncrease();
    void onButtonSizeHorizontalDecrease();

private:
    // void startMeasurementEvent(vtkObject* caller,
    //     long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(callData));
    // void endMeasurementEvent(vtkObject* caller,
    //     long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(callData));
    //for logger prints
    QString getLabelFromOrientation(Orientation orientation) const;
    std::vector<double> getColorFromAxis(VisualAxis axis) const;
    void getAxisAndDirectionFromOrientation(Orientation orientation,
        VisualAxis& axis, bool& direction) const;
    const unit::Millimeters getCrosshairAxisCoord(VisualAxis axis) const;
    double getMinAxisBound(VisualAxis axis);
    double getMaxAxisBound(VisualAxis axis);
    void limitCoordToAllDataBounds(unit::Point& coord);
    void initGui();
    void initCameraDirection();
    void initInteratorStyle();
    void initCrosshair();
    void initAnnotation();

    Orientation m_orientation;
    vtkSmartPointer<vtkImageStack> m_stack;

    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> m_it;
    vtkSmartPointer<SliceViewerInteractorStyle> m_interactorStyle;
    vtkSmartPointer<vtkCornerAnnotation> m_annotation;

    //stores which axis visually corresponds to the axis in world coord
    //is there a better term for the third axis (direction towards the camera)?
    VisualAxis m_horizontalAxis, m_verticalAxis, m_normalAxis;
    vtkSmartPointer<vtkLineWidget2> m_horizontalLine;
    vtkSmartPointer<vtkLineWidget2> m_verticalLine;

    //stores which orientation is at each edge of the viewer
    Orientation m_leftOrientation, m_rightOrientation, m_upperOrientation,
        m_lowerOrientation, m_towardsOrientation, m_backwardsOrientation;

    //vars for measurement
    vtkSmartPointer<vtkDistanceWidget>
        m_distance; // all the distance presentations will use only one pointer
    vtkSmartPointer<vtkAngleWidget> m_angle;
    MeasurementType m_measureType;

    //control widget for surgery model
    vtkSmartPointer<VtkCustomizedButton> m_surgeryModelControlWidget[10];

    QVTKOpenGLWidget* m_openGLWidget;

    double m_defaultCameraViewUp[3];
    //control whether crosshair can be moved by mouse
    bool m_navigation;
    //enum for mouse buttons
    enum Button
    {
        LEFT_MOUSE = 0,
        MIDDLE_MOUSE = 1,
        RIGHT_MOUSE = 2
    };
    //tracks which mouse button is held
    bool m_mouseDown[3];
    //in order of xmin, xmax, ymin, ymax, zmin, zmax
    double m_allDataBounds[6];

    //vars that haven't been used
    bool m_startMeasure; //measure has started for this viewer

    QString m_lowerLeftLabel;
    QMap<int, vtkSmartPointer<vtkBillboardTextActor3D>>
        m_markerPointLabelTextMap; //TODO this variable will be removed after optimizing marker class
};

} // namespace visualization
}; // namespace ultrast