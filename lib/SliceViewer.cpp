#include "ViewerManager.h"
#include "VtkCustomizedButton.h"
#include "vtkCallbackCommand.h"
#include "vtkImageSliceCollection.h"
#include "vtkNrrdReader.h"
#include "vtkSphereSource.h"
#include "vtkTextProperty.h"
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <chrono>
// #include <domain\data\VolumeImageManager.h>
// #include <infrastructure\utility\Logger.h>
// #include <infrastructure\utility\MarkerPoint.h>
// #include <infrastructure\utility\MarkerPointActor.h>
// #include <infrastructure\utility\Model.h>
// #include <infrastructure\utility\PointSet.h>
// #include <infrastructure\utility\Setting.h>
#include <SliceViewer.h>
#include <VolumeImage.h>
#include <iostream>
#include <string>
#include <vector>
#include <vtkRendererCollection.h>

using VolumeImage = ultrast::infrastructure::utility::VolumeImage;

namespace ultrast {
namespace visualization {

SliceViewer::SliceViewer(QWidget* parent, Orientation orientation) :
    BaseViewer(parent, "Slice Viewer"),
    m_orientation(orientation),
    m_startMeasure(false),
    m_measureType(MeasurementType::NONE),
    m_navigation(true),
    m_lowerLeftLabel(""),
    m_renderer(vtkSmartPointer<vtkRenderer>::New()),
    m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New()),
    m_mouseDown { false },
    m_allDataBounds { 0 }
{
    m_type = ViewerType::SLICE;
    initGui();
    initCameraDirection();
    initInteratorStyle();
    initCrosshair();
    initAnnotation();
    // addVtkResizeButton(m_renderer, m_it);
}

SliceViewer::~SliceViewer()
{
}

ViewerInfo SliceViewer::getViewerInfo() const
{
    return ViewerInfo(getViewerId(), m_type, m_normalAxis);
}

const QImage SliceViewer::capture()
{
    this->render();
    return m_openGLWidget->grabFramebuffer();
}

void SliceViewer::update()
{
}

void SliceViewer::bindViewerManager(ViewerManager* viewerManager)
{
    connect(this, &SliceViewer::coordinateChanged, viewerManager,
        &ViewerManager::coordinateChangedHandler);
    connect(viewerManager, &ViewerManager::coordinateChanged,
        [this](
            const unit::Point& coord) { this->setCrosshairCoordinate(coord); });
    // connect(this, &SliceViewer::leftButtonDown, viewerManager, &ViewerManager::leftButtonDown);
    // connect(this, &SliceViewer::middleButtonDown, viewerManager,
    // &ViewerManager::middleButtonDown);
    // connect(this, &SliceViewer::middleButtonUp, viewerManager,
    //     &ViewerManager::middleButtonUp);
    // connect(this, &SliceViewer::rightButtonDown, viewerManager,
    //     &ViewerManager::rightButtonDown);
    // connect(this, &SliceViewer::buttonModifyOrigin, viewerManager,
    //     &ViewerManager::buttonModifyOrigin);
    // connect(this, &SliceViewer::buttonModifyOrientation, viewerManager,
    //     &ViewerManager::buttonModifyOrientation);
    // connect(this, &SliceViewer::buttonModifySize, viewerManager,
    //     &ViewerManager::buttonModifySize);
}

void SliceViewer::addVolumeImage(VolumeImage& volumeImage)
{
    qDebug() << "SliceViewer - adding volume image";
    m_stack->AddImage(volumeImage.getImageSlice(getViewerId()));
    m_stack->GetImages()->Sort();
}

void SliceViewer::removeVolumeImage(VolumeImage& volumeImage)
{
    m_stack->RemoveImage(volumeImage.getImageSlice(getViewerId()));
}

// void SliceViewer::addModel(Model& model)
// {
//     auto coord = getCrosshairCoordinate();
//     model.setCutPosition(coord);
//     connect(this, &SliceViewer::updateSlicedDataCoordinate, &model,
//         &Model::setCutPosition);
//     connect(&model, &Model::positionUpdated, [this, &model]() {
//         model.setCutPosition(this->getCrosshairCoordinate());
//     });
//     m_renderer->AddActor(model.getAssembly(getViewerId()));
// }

// void SliceViewer::addMarker(MarkerPoint& marker)
// {
//     Logger::trace(QString("SliceViewer - addMarker"));
//     auto coord = getCrosshairCoordinate();
//     marker.setCutPosition(coord);

//     connect(this, &SliceViewer::updateSlicedDataCoordinate, &marker,
//         &MarkerPoint::setCutPosition);
//     connect(&marker, &MarkerPoint::positionUpdated, [this, &marker]() {
//         marker.setCutPosition(this->getCrosshairCoordinate());
//     });

//     m_renderer->AddActor(marker.getActor(getViewerId()));
// }

// void SliceViewer::removeMarker(MarkerPoint& marker)
// {
//     m_renderer->RemoveActor(marker.getActor(getViewerId()));
// }

// void SliceViewer::removeModel(Model& model)
// {
//     m_renderer->RemoveActor(model.getAssembly(getViewerId()));
// }

// void SliceViewer::addPointSet(PointSet& pointset)
// {
//     Logger::trace("SliceViewer - adding point set");
//     auto coord = getCrosshairCoordinate();
//     pointset.setCutPosition(coord);
//     connect(this, &SliceViewer::updateSlicedDataCoordinate, &pointset,
//         &PointSet::setCutPosition);
//     m_renderer->AddActor(pointset.getActor(getViewerId()));
// }

// void SliceViewer::removePointSet(PointSet& pointset)
// {
//     m_renderer->RemoveActor(pointset.getActor(getViewerId()));
// }

// void SliceViewer::addImage2d(Image2d& image2d)
// {
// }

// void SliceViewer::removeImage2d(Image2d& image2d)
// {
// }

const unit::Point SliceViewer::getCrosshairCoordinate() const
{
    unit::Point coordinate;
    coordinate[m_horizontalAxis] = getCrosshairAxisCoord(m_horizontalAxis);
    coordinate[m_verticalAxis] = getCrosshairAxisCoord(m_verticalAxis);
    coordinate[m_normalAxis] = getCrosshairAxisCoord(m_normalAxis);
    qDebug() << "SliceViewer - Crosshair coordinates at "
             << coordinate.toQString();
    return coordinate;
}

void SliceViewer::centerCrosshair()
{
    double centerCoord[3];
    centerCoord[0]
        = (getMaxAxisBound(VisualAxis::X) + getMinAxisBound(VisualAxis::X))
        / 2.0;
    centerCoord[1]
        = (getMaxAxisBound(VisualAxis::Y) + getMinAxisBound(VisualAxis::Y))
        / 2.0;
    centerCoord[2]
        = (getMaxAxisBound(VisualAxis::Z) + getMinAxisBound(VisualAxis::Z))
        / 2.0;
    setCrosshairCoordinate(centerCoord);
}

void SliceViewer::setAllDataBounds(double* bounds)
{
    auto equal = [](double a, double b) -> bool {
        return fabs(a - b) < std::numeric_limits<double>::epsilon();
    };
    bool change = false;
    for (int i = 0; i < 6; ++i)
    {
        // value of bounds will change
        if (!change && !equal(m_allDataBounds[i], bounds[i]))
            change = true;
        m_allDataBounds[i] = bounds[i];
    }
    if (change) // bounds has changed, update crosshair size
        refreshCrosshairSize();
}

const VisualAxis SliceViewer::getThirdAxis()
{
    return m_normalAxis;
}

void SliceViewer::setParallelScale(const double& parallelScale)
{
    m_renderer->GetActiveCamera()->SetParallelScale(parallelScale);
}

const unit::Point SliceViewer::getEventCoord() const
{
    // get position of event click
    int* eventPosition = m_it->GetEventPosition();
    // convert eventPosition to world coordinate
    vtkSmartPointer<vtkCoordinate> coordinate
        = vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(eventPosition[0], eventPosition[1], 0);
    double* eventCoord = coordinate->GetComputedWorldValue(
        m_renderWindow->GetRenderers()->GetFirstRenderer());
    unit::Point coord(eventCoord);
    // sometimes value of 3rd axis is wrong, need to fix it to camera focal
    coord[m_normalAxis]
        = m_renderer->GetActiveCamera()->GetFocalPoint()[m_normalAxis];
    return coord;
}

vtkCamera* SliceViewer::getCamera()
{
    return m_renderer->GetActiveCamera();
}

void SliceViewer::setCamera(vtkCamera* camera)
{
    if (camera == nullptr)
    {
        return;
    }

    m_renderer->SetActiveCamera(camera);
}

VisualAxis SliceViewer::getHorizontalAxis()
{
    return m_horizontalAxis;
}

VisualAxis SliceViewer::getVerticalAxis()
{
    return m_verticalAxis;
}

VisualAxis SliceViewer::getNormalAxis()
{
    return m_normalAxis;
}

void SliceViewer::createSlicerViewerWidgets()
{
    void (SliceViewer::*bindFunc[10])() = { &SliceViewer::onButtonPositionUp,
        &SliceViewer::onButtonPositionDown, &SliceViewer::onButtonPositionLeft,
        &SliceViewer::onButtonPositionRight,
        &SliceViewer::onButtonRotationClock,
        &SliceViewer::onButtonRotationAntiClock,
        &SliceViewer::onButtonSizeVerticalIncrease,
        &SliceViewer::onButtonSizeVerticalDecrease,
        &SliceViewer::onButtonSizeHorizontalDecrease,
        &SliceViewer::onButtonSizeHorizontalIncrease };

    for (int i = 0; i < 10; i++)
    {
        m_surgeryModelControlWidget[i]
            = vtkSmartPointer<VtkCustomizedButton>::New();
        m_surgeryModelControlWidget[i]->setupButton(m_renderer, m_it,
            QSize(50, 50), ButtonType::Axis, ButtonAxisSubType(i));
        m_renderer->AddViewProp(
            m_surgeryModelControlWidget[i]->GetRepresentation());
        // bind the corresponding function
        m_surgeryModelControlWidget[i]->AddObserver(
            vtkCommand::StateChangedEvent, this, bindFunc[i]);
        m_renderer->GetRenderWindow()->AddObserver(
            vtkCommand::WindowResizeEvent, m_surgeryModelControlWidget[i],
            &VtkCustomizedButton::repositionOnWindowResize);
        m_surgeryModelControlWidget[i]->SetEnabled(true); // activate the widget
        m_surgeryModelControlWidget[i]->repositionOnWindowResize();
    }
    render();
}

void SliceViewer::deleteSlicerViewerWidgets()
{
    for (int i = 0; i < 10; i++)
    {
        m_renderer->RemoveViewProp(
            m_surgeryModelControlWidget[i]->GetRepresentation());
        // Delete will not be ok since when recreate the instance by calling ::New(), the reference will decrease by 1
        // and it will release for twice
        // m_surgeryModelControlWidget[i]->Delete();

        m_surgeryModelControlWidget[i] = nullptr;
    }
    render();
}

void SliceViewer::zoomIn()
{
    m_interactorStyle->OnMouseWheelForward();
}

void SliceViewer::zoomOut()
{
    m_interactorStyle->OnMouseWheelBackward();
}

void SliceViewer::render()
{
    if (isVisible())
    { //no need to render if widget is hidden
        m_renderer->ResetCameraClippingRange();
        m_renderWindow->Render();
    }
}

void SliceViewer::setFocalPoint(unit::Point focalPoint)
{
    m_focalPoint = focalPoint;
}

void SliceViewer::resetCamera()
{
    double defaultCrosshairLength[] = { 100, 120, 100 };
    double half_length[3] = { defaultCrosshairLength[0] / 2.0,
        defaultCrosshairLength[1] / 2.0, defaultCrosshairLength[2] / 2.0 };
    double bounds[6] = {
        m_focalPoint[0] - half_length[0],
        m_focalPoint[0] + half_length[0],
        m_focalPoint[1] - half_length[1],
        m_focalPoint[1] + half_length[1],
        m_focalPoint[2] - half_length[2],
        m_focalPoint[2] + half_length[2],
    };
    m_renderer->ResetCamera(bounds);
    m_renderer->GetActiveCamera()->SetViewUp(m_defaultCameraViewUp);
    // calculations for parallel scale to control zoom level
    // adapted from https://github.com/Kitware/paraview-glance/issues/230
    double height
        = (bounds[(m_verticalAxis * 2) + 1] - bounds[m_verticalAxis * 2]) / 2;
    double width
        = (bounds[(m_horizontalAxis * 2) + 1] - bounds[m_horizontalAxis * 2])
        / 2;
    double windowWidth = this->width();
    double windowHeight = this->height();
    double ratio = windowWidth / windowHeight;
    if (ratio >= width / height)
        m_renderer->GetActiveCamera()->SetParallelScale(height + 2);
    else
        m_renderer->GetActiveCamera()->SetParallelScale(width / ratio + 2);
    setCrosshairCoordinate(m_focalPoint);
}

void SliceViewer::resetCamera(double bounds[6])
{
    m_renderer->ResetCamera(bounds);
    render();
}

void SliceViewer::centerCamera()
{
    m_renderer->ResetCamera(m_allDataBounds);
    m_renderer->GetActiveCamera()->SetViewUp(m_defaultCameraViewUp);
    // calculations for parallel scale to control zoom level
    // adapted from https://github.com/Kitware/paraview-glance/issues/230
    double height
        = (getMaxAxisBound(m_verticalAxis) - getMinAxisBound(m_verticalAxis))
        / 2;
    double width = (getMaxAxisBound(m_horizontalAxis)
                       - getMinAxisBound(m_horizontalAxis))
        / 2;
    double windowWidth = this->width();
    double windowHeight = this->height();
    double ratio = windowWidth / windowHeight;
    if (ratio >= width / height)
        m_renderer->GetActiveCamera()->SetParallelScale(height + 2);
    else
        m_renderer->GetActiveCamera()->SetParallelScale(width / ratio + 2);

    centerCrosshair();
}

void SliceViewer::setCrosshairCoordinate(const unit::Point& coord, bool relay)
{
    setCameraFocalPoint(coord);

    double coordArray[3];
    // third axis does not change
    // sometimes has overlap issue with image slice
    //  -1 to ensure is is above the slice
    coordArray[m_normalAxis] = coord[m_normalAxis] - 1;
    { // horizontal line
        coordArray[m_verticalAxis] = coord[m_verticalAxis];
        coordArray[m_horizontalAxis] = getMinAxisBound(m_horizontalAxis);
        m_horizontalLine->GetLineRepresentation()->SetPoint1WorldPosition(
            coordArray);
        coordArray[m_horizontalAxis] = getMaxAxisBound(m_horizontalAxis);
        m_horizontalLine->GetLineRepresentation()->SetPoint2WorldPosition(
            coordArray);
    }
    { // vertical line
        coordArray[m_horizontalAxis] = coord[m_horizontalAxis];
        coordArray[m_verticalAxis] = getMinAxisBound(m_verticalAxis);
        m_verticalLine->GetLineRepresentation()->SetPoint1WorldPosition(
            coordArray);
        coordArray[m_verticalAxis] = getMaxAxisBound(m_verticalAxis);
        m_verticalLine->GetLineRepresentation()->SetPoint2WorldPosition(
            coordArray);
    }

    // this parameter
    if (relay)
        emit coordinateChanged(coord);
    else
    {
        emit updateSlicedDataCoordinate(coord);
        render();
    }
}

void SliceViewer::refreshCrosshairSize()
{
    auto coord = getCrosshairCoordinate();
    setCrosshairCoordinate(coord);
}

void SliceViewer::setCameraFocalPoint(const unit::Point& coord)
{
    double focalPoint[3];
    m_renderer->GetActiveCamera()->GetFocalPoint(focalPoint);
    focalPoint[m_normalAxis] = coord[m_normalAxis];
    m_renderer->GetActiveCamera()->SetFocalPoint(focalPoint);
}

// void SliceViewer::disableMeasurement(int measurementType)
// {
//     qDebug()
//         << "SliceViewer - disable measurement slot called for orientation: "
//         << getOrientationAsQString();
//     // Logger::trace(QString(
//     //     "SliceViewer - disable measurement slot called for orientation: %1")
//     //                   .arg(getOrientationAsQString()));
//     m_startMeasure = false;
//     switch (measurementType)
//     {
//     case MeasurementType::DISTANCE:
//         m_distance->Off();
//         m_distance = nullptr;
//         break;
//     case MeasurementType::ANGLE:
//         m_angle->Off();
//         m_angle = nullptr;
//         break;
//     }
//     //m_measureType = MeasurementType::NONE;
// }

// void SliceViewer::listenDistanceMeasurement(bool enable)
// {
//     if (enable)
//     {
//         Logger::trace(QString(
//             "SliceViewer - listening for distance measurement for orientation: %1")
//                           .arg(getOrientationAsQString()));
//         m_measureType = MeasurementType::DISTANCE;
//         if (!m_distance)
//         {
//             m_distance = vtkSmartPointer<vtkDistanceWidget>::New();
//             m_distance->SetInteractor(m_it);
//             m_distance->CreateDefaultRepresentation();
//             m_distance->GetDistanceRepresentation()->SetLabelFormat(
//                 "%-#.2f mm");
//             m_distance->AddObserver(vtkCommand::EndInteractionEvent, this,
//                 &SliceViewer::endMeasurementEvent);
//             m_distance->On();
//         }
//         else
//         {
//             m_distance->On();
//         }
//     }
//     else
//     {
//         disableMeasurement(1);
//     }
// }

// void SliceViewer::listenAngleMeasurement(bool enable)
// {
//     if (enable)
//     {
//         Logger::trace(QString(
//             "SliceViewer - listening for angle measurement for orientation: %1")
//                           .arg(getOrientationAsQString()));
//         m_measureType = MeasurementType::ANGLE;
//         if (!m_angle)
//         {
//             m_angle = vtkSmartPointer<vtkAngleWidget>::New();
//             m_angle->SetInteractor(m_it);
//             m_angle->CreateDefaultRepresentation();
//             m_angle->GetAngleRepresentation()->SetLabelFormat("%#.2f");
//             m_angle->AddObserver(vtkCommand::EndInteractionEvent, this,
//                 &SliceViewer::endMeasurementEvent);
//             m_angle->On();
//         }
//         else
//         {
//             m_angle->On();
//         }
//     }
//     else
//     {
//         disableMeasurement(2);
//     }
// }

// void SliceViewer::listenVolumeMeasurement(bool enable)
// {
//     if (enable)
//     {
//         Logger::trace(QString(
//             "SliceViewer - listening for volume measurement for orientation: %1")
//                           .arg(getOrientationAsQString()));
//         m_measureType = MeasurementType::VOLUME;
//         m_startMeasure = true;
//     }
//     else
//     {
//         m_measureType = MeasurementType::NONE;
//         m_startMeasure = false;
//     }
// }

void SliceViewer::leftClickDown()
{
    m_mouseDown[Button::LEFT_MOUSE] = true;
    auto coord = getEventCoord();
    qDebug() << "SliceViewer - Left click pressed at " << coord.toQString();
    if (m_navigation)
    {
        limitCoordToAllDataBounds(coord);
        setCrosshairCoordinate(coord, true);
        emit coordinateChangedByMouse(coord);
    }
    emit leftButtonDown(coord);
    emit leftClick(m_lowerLeftLabel, coord);
}

void SliceViewer::leftClickUp()
{
    m_mouseDown[Button::LEFT_MOUSE] = false;
    auto coord = getEventCoord();
    qDebug() << "SliceViewer - Left click released at " << coord.toQString();
    emit leftButtonUp(coord);
}

void SliceViewer::leftDrag()
{
    auto coord = getEventCoord();
    if (m_mouseDown[Button::LEFT_MOUSE])
    {
        if (m_navigation)
        {
            limitCoordToAllDataBounds(coord);
            setCrosshairCoordinate(coord, true);
            emit coordinateChangedByMouse(coord);
        }
    }
    emit mouseMove(m_lowerLeftLabel, coord);
}

void SliceViewer::rightClickDown()
{
    m_mouseDown[Button::RIGHT_MOUSE] = true;
    auto coord = getEventCoord();
    qDebug() << "SliceViewer - Right click pressed at " << coord.toQString();

    emit rightButtonDown(coord);
}

void SliceViewer::rightClickUp()
{
    m_mouseDown[Button::RIGHT_MOUSE] = false;
    auto coord = getEventCoord();
    qDebug() << "SliceViewer - Right click released at " << coord.toQString();

    emit rightButtonUp(coord);
}

void SliceViewer::middleClickDown()
{
    m_mouseDown[Button::MIDDLE_MOUSE] = true;
    auto coord = getEventCoord();
    qDebug() << "SliceViewer - Middle click pressed at " << coord.toQString();
    emit middleButtonDown(coord);
}

void SliceViewer::middleClickUp()
{
    m_mouseDown[Button::MIDDLE_MOUSE] = false;
    auto coord = getEventCoord();
    qDebug() << "SliceViewer - Middle click released at " << coord.toQString();
    emit middleButtonUp(coord);
}

void SliceViewer::onButtonPositionUp()
{
    VisualAxis axis;
    bool direction;
    getAxisAndDirectionFromOrientation(m_upperOrientation, axis, direction);
    emit buttonModifyOrigin(axis, direction);
}

void SliceViewer::onButtonPositionDown()
{
    VisualAxis axis;
    bool direction;
    getAxisAndDirectionFromOrientation(m_lowerOrientation, axis, direction);
    emit buttonModifyOrigin(axis, direction);
}

void SliceViewer::onButtonPositionLeft()
{
    VisualAxis axis;
    bool direction;
    getAxisAndDirectionFromOrientation(m_leftOrientation, axis, direction);
    emit buttonModifyOrigin(axis, direction);
}

void SliceViewer::onButtonPositionRight()
{
    VisualAxis axis;
    bool direction;
    getAxisAndDirectionFromOrientation(m_rightOrientation, axis, direction);
    emit buttonModifyOrigin(axis, direction);
}

void SliceViewer::onButtonRotationAntiClock()
{
    VisualAxis axis;
    bool direction;
    getAxisAndDirectionFromOrientation(m_towardsOrientation, axis, direction);
    emit buttonModifyOrientation(axis, direction);
}

void SliceViewer::onButtonRotationClock()
{
    VisualAxis axis;
    bool direction;
    getAxisAndDirectionFromOrientation(m_backwardsOrientation, axis, direction);
    emit buttonModifyOrientation(axis, direction);
}

void SliceViewer::onButtonSizeVerticalIncrease()
{
    emit buttonModifySize(m_verticalAxis, true);
}

void SliceViewer::onButtonSizeVerticalDecrease()
{
    emit buttonModifySize(m_verticalAxis, false);
}

void SliceViewer::onButtonSizeHorizontalIncrease()
{
    emit buttonModifySize(m_horizontalAxis, true);
}

void SliceViewer::onButtonSizeHorizontalDecrease()
{
    emit buttonModifySize(m_horizontalAxis, false);
}

// void SliceViewer::startMeasurementEvent(vtkObject* caller,
//     long unsigned int eventId, void* callData)
// {
//     // this method also gets called when modifying existing measurements, which will call the error logs but no crash/memory issues
//     m_startMeasure = true;
//     Logger::trace(
//         QString("SliceViewer - measurement chosen viewer orientation: %1")
//             .arg(getOrientationAsQString()));
//     // disable measure for other viewers
//     Logger::trace(
//         "SliceViewer - emit signal to disable listening in other SliceViewers");
//     emit measurementStarted();
// }

// void SliceViewer::endMeasurementEvent(vtkObject* caller,
//     long unsigned int eventId, void* callData)
// {
//     Logger::trace("SliceViewer - pass widget to be stored for measurement");
//     switch (m_measureType)
//     {
//     case MeasurementType::DISTANCE:
//         m_distance->ProcessEventsOff();
//         emit storeDistanceWidget(m_distance, m_normalAxis,
//             getCrosshairAxisCoord(m_normalAxis)); //store the distance widgets
//         m_distance = nullptr;

//         //recreate a new instance for next usage
//         m_distance = vtkSmartPointer<vtkDistanceWidget>::New();
//         m_distance->SetInteractor(m_it);
//         m_distance->CreateDefaultRepresentation();
//         m_distance->GetDistanceRepresentation()->SetLabelFormat("%-#.2f mm");
//         m_distance->AddObserver(vtkCommand::EndInteractionEvent, this,
//             &SliceViewer::endMeasurementEvent);
//         m_distance->On();
//         break;
//     case MeasurementType::ANGLE:
//         m_angle->ProcessEventsOff();
//         emit storeAngleWidget(m_angle, m_normalAxis,
//             getCrosshairAxisCoord(m_normalAxis));
//         m_angle = nullptr;

//         m_angle = vtkSmartPointer<vtkAngleWidget>::New();
//         m_angle->SetInteractor(m_it);
//         m_angle->CreateDefaultRepresentation();
//         m_angle->GetAngleRepresentation()->SetLabelFormat("%#.2f");
//         m_angle->AddObserver(vtkCommand::EndInteractionEvent, this,
//             &SliceViewer::endMeasurementEvent);
//         m_angle->On();
//         break;
//     default:
//         Logger::error(tr("SliceViewer - Measure is on but there is no type"));
//         break;
//     }
// }

QString SliceViewer::getOrientationAsQString() const
{
    QString result = "NULL";
    switch (m_orientation)
    {
    case Orientation::LEFT:
        result = "LEFT";
        break;
    case Orientation::RIGHT:
        result = "RIGHT";
        break;
    case Orientation::SUPERIOR:
        result = "SUPERIOR";
        break;
    case Orientation::INFERIOR:
        result = "INFERIOR";
        break;
    case Orientation::ANTERIOR:
        result = "ANTERIOR";
        break;
    case Orientation::POSTERIOR:
        result = "POSTERIOR";
        break;
    }
    return result;
}

QString SliceViewer::getLabelFromOrientation(Orientation orientation) const
{
    QString result;
    switch (orientation)
    {
    case Orientation::LEFT:
        result = "L";
        break;
    case Orientation::RIGHT:
        result = "R";
        break;
    case Orientation::SUPERIOR:
        result = "S";
        break;
    case Orientation::INFERIOR:
        result = "I";
        break;
    case Orientation::ANTERIOR:
        result = "A";
        break;
    case Orientation::POSTERIOR:
        result = "P";
        break;
    default:
        break;
    }
    return result;
}

std::vector<double> SliceViewer::getColorFromAxis(VisualAxis axis) const
{
    std::vector<double> color = { 0, 0, 0 };
    switch (axis)
    {
    case VisualAxis::X:
        color[1] = 1;
        break;
    case VisualAxis::Y:
        color[2] = 1;
        break;
    case VisualAxis::Z:
        color[0] = 1;
    default:
        break;
    }
    return color;
}

void SliceViewer::getAxisAndDirectionFromOrientation(Orientation orientation,
    VisualAxis& axis, bool& direction) const
{
    switch (orientation)
    {
    case Orientation::LEFT:
        axis = VisualAxis::X;
        direction = true;
        break;
    case Orientation::RIGHT:
        axis = VisualAxis::X;
        direction = false;
        break;
    case Orientation::SUPERIOR:
        axis = VisualAxis::Z;
        direction = true;
        break;
    case Orientation::INFERIOR:
        axis = VisualAxis::Z;
        direction = false;
        break;
    case Orientation::ANTERIOR:
        axis = VisualAxis::Y;
        direction = false;
        break;
    case Orientation::POSTERIOR:
        axis = VisualAxis::Y;
        direction = true;
        break;
    default:
        break;
    }
}

const unit::Millimeters SliceViewer::getCrosshairAxisCoord(
    VisualAxis axis) const
{
    if (m_horizontalAxis == axis)
        return m_verticalLine->GetLineRepresentation()
            ->GetPoint1WorldPosition()[axis];
    else if (m_verticalAxis == axis)
        return m_horizontalLine->GetLineRepresentation()
            ->GetPoint1WorldPosition()[axis];
    else if (m_normalAxis == axis)
        return m_renderer->GetActiveCamera()->GetFocalPoint()[axis];
    else
        return 0.0;
}

double SliceViewer::getMinAxisBound(VisualAxis axis)
{
    return m_allDataBounds[axis * 2];
}

double SliceViewer::getMaxAxisBound(VisualAxis axis)
{
    return m_allDataBounds[axis * 2 + 1];
}

void SliceViewer::limitCoordToAllDataBounds(unit::Point& coord)
{
    VisualAxis axes[] = { m_horizontalAxis, m_verticalAxis, m_normalAxis };
    for (auto axis : axes)
        if (coord[axis] > getMaxAxisBound(axis))
            coord[axis] = getMaxAxisBound(axis);
        else if (coord[axis] < getMinAxisBound(axis))
            coord[axis] = getMinAxisBound(axis);
}

void SliceViewer::initGui()
{
    auto layout = new QVBoxLayout(this);
    m_openGLWidget = new QVTKOpenGLWidget(this);
    layout->addWidget(m_openGLWidget);

    m_renderWindow->AddRenderer(m_renderer);
    m_openGLWidget->SetRenderWindow(m_renderWindow);

    m_stack = vtkSmartPointer<vtkImageStack>::New();
    m_renderer->AddViewProp(m_stack);
}

void SliceViewer::initCameraDirection()
{
    switch (m_orientation)
    {
    case Orientation::LEFT:
        m_renderer->GetActiveCamera()->Yaw(-90);
        m_renderer->GetActiveCamera()->Roll(180);
        m_horizontalAxis = VisualAxis::Z;
        m_verticalAxis = VisualAxis::Y;
        m_normalAxis = VisualAxis::X;
        m_leftOrientation = SUPERIOR;
        m_rightOrientation = INFERIOR;
        m_upperOrientation = ANTERIOR;
        m_lowerOrientation = POSTERIOR;
        m_towardsOrientation = RIGHT;
        m_backwardsOrientation = LEFT;
        m_lowerLeftLabel = "SAG";
        break;
    case Orientation::RIGHT:

        m_renderer->GetActiveCamera()->Yaw(90);
        m_renderer->GetActiveCamera()->Roll(180);
        m_horizontalAxis = VisualAxis::Z;
        m_verticalAxis = VisualAxis::Y;
        m_normalAxis = VisualAxis::X;
        m_leftOrientation = INFERIOR;
        m_rightOrientation = SUPERIOR;
        m_upperOrientation = ANTERIOR;
        m_lowerOrientation = POSTERIOR;
        m_towardsOrientation = LEFT;
        m_backwardsOrientation = RIGHT;
        break;
    case Orientation::SUPERIOR:
        m_renderer->GetActiveCamera()->Roll(180);
        m_renderer->GetActiveCamera()->Azimuth(180);
        m_horizontalAxis = VisualAxis::X;
        m_verticalAxis = VisualAxis::Y;
        m_normalAxis = VisualAxis::Z;
        m_leftOrientation = RIGHT;
        m_rightOrientation = LEFT;
        m_upperOrientation = ANTERIOR;
        m_lowerOrientation = POSTERIOR;
        m_towardsOrientation = INFERIOR;
        m_backwardsOrientation = SUPERIOR;
        m_lowerLeftLabel = "TRA";
        break;
    case Orientation::INFERIOR:
        m_renderer->GetActiveCamera()->Roll(180);
        m_horizontalAxis = VisualAxis::X;
        m_verticalAxis = VisualAxis::Y;
        m_normalAxis = VisualAxis::Z;
        m_leftOrientation = LEFT;
        m_rightOrientation = RIGHT;
        m_upperOrientation = ANTERIOR;
        m_lowerOrientation = POSTERIOR;
        m_towardsOrientation = SUPERIOR;
        m_backwardsOrientation = INFERIOR;
        break;
    case Orientation::ANTERIOR:
        m_renderer->GetActiveCamera()->Pitch(-90);
        m_renderer->GetActiveCamera()->Roll(180);
        m_horizontalAxis = VisualAxis::X;
        m_verticalAxis = VisualAxis::Z;
        m_normalAxis = VisualAxis::Y;
        m_leftOrientation = LEFT;
        m_rightOrientation = RIGHT;
        m_upperOrientation = SUPERIOR;
        m_lowerOrientation = INFERIOR;
        m_towardsOrientation = POSTERIOR;
        m_backwardsOrientation = ANTERIOR;
        break;
    case Orientation::POSTERIOR:

        m_renderer->GetActiveCamera()->Pitch(90);
        m_horizontalAxis = VisualAxis::X;
        m_verticalAxis = VisualAxis::Z;
        m_normalAxis = VisualAxis::Y;
        m_leftOrientation = RIGHT;
        m_rightOrientation = LEFT;
        m_upperOrientation = SUPERIOR;
        m_lowerOrientation = INFERIOR;
        m_towardsOrientation = ANTERIOR;
        m_backwardsOrientation = POSTERIOR;
        m_lowerLeftLabel = "COR";
        break;
    default:
        break;
    }
    m_renderer->GetActiveCamera()->OrthogonalizeViewUp();
    m_renderer->GetActiveCamera()->ParallelProjectionOn();

    // store the default values for reseting camera
    std::copy(m_renderer->GetActiveCamera()->GetViewUp(),
        m_renderer->GetActiveCamera()->GetViewUp() + 3, m_defaultCameraViewUp);
    qDebug() << "SliceViewer - Camera GetViewUp(" << m_defaultCameraViewUp[0]
             << ", " << m_defaultCameraViewUp[1] << ", "
             << m_defaultCameraViewUp[2] << ")";
    // Logger::trace(QString("SliceViewer - Camera GetViewUp(%1, %2, %3)")
    //                   .arg(m_defaultCameraViewUp[0])
    //                   .arg(m_defaultCameraViewUp[1])
    //                   .arg(m_defaultCameraViewUp[2]));
}

void SliceViewer::initInteratorStyle()
{
    // After QVTKOpenGLWidget::SetRenderWindow(), we can get a non-empty pointer, otherwise it's nullptr
    m_it = m_renderWindow->GetInteractor();
    // interactor style for mouse controls
    m_interactorStyle = vtkSmartPointer<SliceViewerInteractorStyle>::New();
    m_interactorStyle->SetRenderer(m_renderer);
    m_interactorStyle->SetRenderWindow(m_renderWindow);
    m_interactorStyle->SetSliceViewer(this);
    m_interactorStyle->SetInteractionModeToImageSlicing();
    m_it->SetInteractorStyle(m_interactorStyle);
}

void SliceViewer::initCrosshair()
{
    // lambda function to reduce duplicated code
    auto setupCrosshairLine =
        [](vtkSmartPointer<vtkLineWidget2>& line, std::vector<double> color,
            vtkSmartPointer<vtkRenderWindowInteractor>& interactor) {
            line = vtkSmartPointer<vtkLineWidget2>::New();
            line->CreateDefaultRepresentation();
            line->GetLineRepresentation()->GetLineProperty()->SetLineWidth(1.5);
            line->GetLineRepresentation()->SetHandleSize(1);
            line->GetLineRepresentation()->SetLineColor(color[0], color[1],
                color[2]);
            line->SetInteractor(interactor);
            line->ProcessEventsOff();
        };

    setupCrosshairLine(m_horizontalLine, getColorFromAxis(m_verticalAxis),
        m_it);
    setupCrosshairLine(m_verticalLine, getColorFromAxis(m_horizontalAxis),
        m_it);

    m_horizontalLine->SetEnabled(true);
    m_verticalLine->SetEnabled(true);
}

void SliceViewer::initAnnotation()
{
    m_annotation = vtkSmartPointer<vtkCornerAnnotation>::New();
    m_annotation->SetLinearFontScaleFactor(2);
    m_annotation->SetNonlinearFontScaleFactor(1);
    m_annotation->SetMaximumFontSize(20);
    m_annotation->GetTextProperty()->SetBold(true);
    m_annotation->GetTextProperty()->SetOpacity(1.0);

    m_annotation->SetText(vtkCornerAnnotation::TextPosition::LeftEdge,
        getLabelFromOrientation(m_leftOrientation).toStdString().c_str());
    m_annotation->SetText(vtkCornerAnnotation::TextPosition::RightEdge,
        getLabelFromOrientation(m_rightOrientation).toStdString().c_str());
    m_annotation->SetText(vtkCornerAnnotation::TextPosition::UpperEdge,
        getLabelFromOrientation(m_upperOrientation).toStdString().c_str());
    m_annotation->SetText(vtkCornerAnnotation::TextPosition::LowerEdge,
        getLabelFromOrientation(m_lowerOrientation).toStdString().c_str());
    if (!m_lowerLeftLabel.isEmpty())
        m_annotation->SetText(vtkCornerAnnotation::TextPosition::LowerLeft,
            m_lowerLeftLabel.toStdString().c_str());
    else
        m_annotation->SetText(vtkCornerAnnotation::TextPosition::LowerLeft,
            getLabelFromOrientation(m_towardsOrientation)
                .toStdString()
                .c_str());
    m_annotation->GetTextProperty()->SetColor(
        getColorFromAxis(m_normalAxis).data());

    m_renderer->AddViewProp(m_annotation);
}

QString SliceViewer::getLabel() const
{
    return m_lowerLeftLabel;
}

void SliceViewer::zoomWithFactor(const double zoomFactor)
{
    m_renderer->GetActiveCamera()->Zoom(zoomFactor);
}

SliceViewerInteractorStyle* SliceViewerInteractorStyle::New()
{
    return new SliceViewerInteractorStyle();
}

void SliceViewerInteractorStyle::SetRenderer(vtkRenderer* renderer)
{
    if (renderer == nullptr)
    {
        return;
    }

    m_renderer = renderer;
}

void SliceViewerInteractorStyle::SetRenderWindow(vtkRenderWindow* renderWindow)
{
    if (renderWindow == nullptr)
    {
        return;
    }

    m_renderWindow = renderWindow;
}

void SliceViewerInteractorStyle::SetSliceViewer(SliceViewer* sliceViewer)
{
    if (sliceViewer == nullptr)
    {
        return;
    }

    m_sliceViewer = sliceViewer;
}

void SliceViewerInteractorStyle::OnMouseMove()
{
    m_sliceViewer->leftDrag();
    vtkInteractorStyleImage::OnMouseMove();
}

void SliceViewerInteractorStyle::OnLeftButtonDown()
{
    m_sliceViewer->leftClickDown();
}

void SliceViewerInteractorStyle::OnLeftButtonUp()
{
    m_sliceViewer->leftClickUp();
}

void SliceViewerInteractorStyle::OnMiddleButtonDown()
{
    m_sliceViewer->middleClickDown();
    vtkInteractorStyleImage::OnMiddleButtonDown();
}

void SliceViewerInteractorStyle::OnMiddleButtonUp()
{
    m_sliceViewer->middleClickUp();
    vtkInteractorStyleImage::OnMiddleButtonUp();
}

void SliceViewerInteractorStyle::OnRightButtonDown()
{
    m_sliceViewer->rightClickDown();
    vtkInteractorStyleImage::OnRightButtonDown();
}

void SliceViewerInteractorStyle::OnRightButtonUp()
{
    m_sliceViewer->rightClickUp();
    vtkInteractorStyleImage::OnRightButtonUp();
}

void SliceViewerInteractorStyle::OnMouseWheelForward()
{
    zoom(true);
}

void SliceViewerInteractorStyle::OnMouseWheelBackward()
{
    zoom(false);
}

void SliceViewerInteractorStyle::zoom(const bool& isIn)
{
    qDebug() << "SliceViewer - zoom slot called for orientation: "
             << m_sliceViewer->getOrientationAsQString();
    // Logger::trace(QString("SliceViewer - %1").arg(m_sliceViewer->getLabel()));
    const unit::Point crosshairPoint = m_sliceViewer->getCrosshairCoordinate();
    double point[3]
        = { crosshairPoint[0], crosshairPoint[1], crosshairPoint[2] };

    double crosshairPosDisplay[3] = { 0 };
    vtkInteractorObserver::ComputeWorldToDisplay(m_renderer, point[0], point[1],
        point[2], crosshairPosDisplay);

    qDebug() << "Position of crosshair in display coordinates before zoom: (x: "
             << crosshairPosDisplay[0] << ", y: " << crosshairPosDisplay[1]
             << ", z: " << crosshairPosDisplay[2] << ")";

    double* fp;
    double* pos;
    vtkCamera* camera = m_renderer->GetActiveCamera();
    fp = camera->GetFocalPoint();
    pos = camera->GetPosition();

    double worldDistance[3] = { 0 };
    vtkMath::Subtract(point, fp, worldDistance);

    double translate[3]
        = { worldDistance[0], worldDistance[1], worldDistance[2] };

    double factor = isIn ? 1 - (1 / m_scaleFactor) : -(m_scaleFactor - 1);
    vtkMath::MultiplyScalar(translate, factor);

    double newFp[3] = { 0 };
    vtkMath::Add(fp, translate, newFp);

    camera->SetFocalPoint(newFp);

    double newPos[3] = { 0 };
    newPos[m_sliceViewer->getHorizontalAxis()]
        = newFp[m_sliceViewer->getHorizontalAxis()];
    newPos[m_sliceViewer->getVerticalAxis()]
        = newFp[m_sliceViewer->getVerticalAxis()];
    newPos[m_sliceViewer->getNormalAxis()]
        = pos[m_sliceViewer->getNormalAxis()];

    camera->SetPosition(newPos);

    double zoomFactor = isIn ? m_scaleFactor : 1 / m_scaleFactor;
    camera->Zoom(zoomFactor);

    vtkInteractorObserver::ComputeWorldToDisplay(m_renderer, point[0], point[1],
        point[2], crosshairPosDisplay);
    qDebug() << "Position of crosshair in display coordinates after zoom: (x: "
             << crosshairPosDisplay[0] << ", y: " << crosshairPosDisplay[1]
             << ", z: " << crosshairPosDisplay[2] << ")";

    m_renderWindow->Render();

    emit m_sliceViewer->zoomChanged(zoomFactor);
}

SliceViewerInteractorStyle::SliceViewerInteractorStyle() :
    m_renderer(nullptr),
    m_renderWindow(nullptr),
    m_sliceViewer(nullptr),
    m_scaleFactor(1.1)
{
}

} // namespace visualization
}; // namespace ultrast