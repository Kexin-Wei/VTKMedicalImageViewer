#include "QuadViewerWidget.h"
// #include "FlexibleSliceViewer.h"
#include "SliceViewer.h"
#include "StereoViewer.h"
#include "ViewerManager.h"
// #include "Volume3DMeasurementWidget.h"
// #include <domain\data\DataManager.h>
// #include <domain\data\VolumeImageManager.h>
#include <Constants.h>
#include <Data3d.h>
// #include <Logger.h>
// #include <Setting.h>
#include <VolumeImage.h>

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QSplitter>
#include <QVBoxLayout>

// using Setting = ultrast::infrastructure::utility::Setting;
using Data3d = ultrast::infrastructure::utility::Data3d;
// using Logger = ultrast::infrastructure::utility::Logger;
// using VolumeImageManager = ultrast::domain::data::VolumeImageManager;

namespace ultrast {
namespace visualization {

QuadViewerWidget::QuadViewerWidget(QWidget* parent,
    const QString& coordinateId) :
    QWidget(parent),
    m_axialViewer(new SliceViewer(this, SliceViewer::SUPERIOR)),
    m_coronalViewer(new SliceViewer(this, SliceViewer::POSTERIOR)),
    m_sagittalViewer(new SliceViewer(this, SliceViewer::LEFT)),
    m_stereoViewer(new StereoViewer(this)),
    m_mainSplitter(new QSplitter(Qt::Vertical)),
    m_topSplitter(new QSplitter()),
    m_middleSplitter(new QSplitter()),
    m_bottomSplitter(new QSplitter()),
    m_viewerManager(new ViewerManager(this)),
    // m_dataManager(DataManager::getInstance()),
    m_allDataBounds { 0 }
// m_flexibleSliceViewer(
//     new FlexibleSliceViewer(this, coordinateId, 150, 100, false, false))
// m_3dVolumeMeasurer(new Volume3DMeasurementWidget(this))
{
    this->initializeUI();

    m_sliceViewers << m_axialViewer << m_coronalViewer << m_sagittalViewer;
    m_stereoViewers << m_stereoViewer;
    m_activeViewers << m_axialViewer << m_coronalViewer << m_sagittalViewer;

    // this->setAllDataBoundsToDefault();
    this->setAllDataBoundsToSliceViewers();
    this->bindConnections();

    m_viewerManager->registerViewer(m_axialViewer);
    m_viewerManager->registerViewer(m_coronalViewer);
    m_viewerManager->registerViewer(m_sagittalViewer);
    m_viewerManager->registerViewer(m_stereoViewer);
    // m_viewerManager->registerViewer(m_flexibleSliceViewer);
    // m_viewerManager->resetCamera();
}

QuadViewerWidget::~QuadViewerWidget()
{
}

void QuadViewerWidget::setFocalPoint(unit::Point point)
{
    m_viewerManager->setFocalPoint(point);
}

void QuadViewerWidget::setAllDataBounds(double* bounds)
{
    m_viewerManager->setAllDataBounds(bounds);
}

void QuadViewerWidget::resetCamera()
{
    m_viewerManager->resetCamera();
}

void QuadViewerWidget::resetCamera(double bounds[6])
{
    m_viewerManager->resetCamera(bounds);
}

void QuadViewerWidget::createSlicerViewerWidgets()
{
    for (int i = 0; i < m_sliceViewers.size(); ++i)
    {
        m_sliceViewers[i]->createSlicerViewerWidgets();
    }
}

void QuadViewerWidget::deleteSlicerViewerWidgets()
{
    for (int i = 0; i < m_sliceViewers.size(); ++i)
    {
        m_sliceViewers[i]->deleteSlicerViewerWidgets();
    }
}

// void QuadViewerWidget::listenForDistanceMeasurement(bool enable)
// {
//     for (int i = 0; i < m_sliceViewers.size(); ++i)
//     {
//         m_sliceViewers[i]->listenDistanceMeasurement(enable);
//     }
// }

// void QuadViewerWidget::listenForAngleMeasurement(bool enable)
// {
//     for (int i = 0; i < m_sliceViewers.size(); ++i)
//     {
//         m_sliceViewers[i]->listenAngleMeasurement(enable);
//     }
// }

// void QuadViewerWidget::listenForVolumeMeasurement(bool enable)
// {
//     m_3dVolumeMeasurer->setEnterVolumeMeasurement(enable);
// }

void QuadViewerWidget::setCrosshairCoordinate(const unit::Point& coord)
{
    for (auto it = m_sliceViewers.begin(); it < m_sliceViewers.end(); ++it)
        (*it)->setCrosshairCoordinate(coord);

    for (auto stereoViewer : m_stereoViewers)
        stereoViewer->storeCoordinate(coord);

    m_viewerManager->setCrosshairCoordinate(coord);
}

void QuadViewerWidget::render()
{
    m_viewerManager->render();
}

const unit::Point QuadViewerWidget::getCrosshairCoordinate()
{
    return m_viewerManager->getCrosshairCoordinate();
}

void QuadViewerWidget::setStereoViewerVisible(const bool& isVisible)
{
    // resetMaxSizedViewer();
    if (isVisible)
    {
        m_activeViewers.insert(m_stereoViewer);
        m_stereoViewer->show();
    }
    else
    {
        m_activeViewers.remove(m_stereoViewer);
        m_stereoViewer->hide();
    }
}

// void QuadViewerWidget::filterOutVolumeImage(
//     const std::vector<VolumeImage*>& images)
// {
//     std::vector<int> idList;
//     for (auto it = images.begin(); it != images.end(); ++it)
//     {
//         idList.push_back((*it)->getId());
//     }
//     m_viewerManager->filterOutVolumeImageById(idList);
// }

// void QuadViewerWidget::resetVolumeImageFilters()
// {
//     m_viewerManager->resetVolumeImageFilters();
// }

void QuadViewerWidget::addData(Data3d& data)
{
    m_viewerManager->addData(data);
    double dataBounds[6];
    data.getBounds(dataBounds);
    for (int i = 0; i < 6; ++i)
    {
        if (i % 2 == 0)
            m_allDataBounds[i] = std::min(m_allDataBounds[i], dataBounds[i]);
        else
            m_allDataBounds[i] = std::max(m_allDataBounds[i], dataBounds[i]);
    }
    for (int i = 0; i < m_sliceViewers.size(); ++i)
        m_sliceViewers[i]->setAllDataBounds(m_allDataBounds);
    render();
}

void QuadViewerWidget::removeData(Data3d& data)
{
    m_viewerManager->removeData(data);
}

bool QuadViewerWidget::isStereoViewerVisible()
{
    return m_stereoViewer->isVisible();
}

bool QuadViewerWidget::hasStereoViewer()
{
    return m_activeViewers.find(m_stereoViewer) != m_activeViewers.end();
}

// void QuadViewerWidget::resetMaxSizedViewer()
// {
//     for (auto viewer : m_activeViewers)
//     {
//         if (viewer->IsMaximized())
//         {
//             viewer->resizeSelf();
//             break;
//         }
//     }
// }

SliceViewer* QuadViewerWidget::getViewerByLable(QString label)
{
    if (label == "TRA")
    {
        return m_axialViewer;
    }
    else if (label == "COR")
    {
        return m_coronalViewer;
    }
    else if (label == "SAG")
    {
        return m_sagittalViewer;
    }
    else
        return nullptr;
}

void QuadViewerWidget::registerExternalStereoViewer(StereoViewer* stereoViewer)
{
    m_stereoViewers.push_back(stereoViewer);
    m_viewerManager->registerViewer(stereoViewer);
}

// void QuadViewerWidget::setFlexibleSliceViewerVisible(const bool& isVisible)
// {
//     if (isVisible)
//     {
//         m_activeViewers.insert(m_flexibleSliceViewer);
//         m_flexibleSliceViewer->show();
//     }
//     else
//     {
//         m_activeViewers.remove(m_flexibleSliceViewer);
//         m_flexibleSliceViewer->hide();
//     }
// }

// void QuadViewerWidget::setImageHorizontalFlip(const bool& horizontalFlip)
// {
//     m_flexibleSliceViewer->setImageHorizontalFlip(horizontalFlip);
// }

// void QuadViewerWidget::setImageVerticalFlip(const bool& verticalFlip)
// {
//     m_flexibleSliceViewer->setImageVerticalFlip(verticalFlip);
// }

void QuadViewerWidget::resetCameraAndSetCrosshair(double bounds[6],
    const unit::Point& point)
{
    resetCamera(bounds);
    setCrosshairCoordinate(point);
}

vtkCamera* QuadViewerWidget::getAxialViewerCamera()
{
    return m_axialViewer->getCamera();
}

vtkCamera* QuadViewerWidget::getCoronalViewerCamera()
{
    return m_coronalViewer->getCamera();
}

vtkCamera* QuadViewerWidget::getSagittalViewerCamera()
{
    return m_sagittalViewer->getCamera();
}

vtkCamera* QuadViewerWidget::getStereoViewerCamera()
{
    return m_stereoViewer->getCamera();
}

void QuadViewerWidget::setAxialViewerCamera(vtkCamera* camera)
{
    m_axialViewer->setCamera(camera);
}

void QuadViewerWidget::setCoronalViewerCamera(vtkCamera* camera)
{
    m_coronalViewer->setCamera(camera);
}

void QuadViewerWidget::setSagittalViewerCamera(vtkCamera* camera)
{
    m_sagittalViewer->setCamera(camera);
}

void QuadViewerWidget::setStereoViewerCamera(vtkCamera* camera)
{
    m_stereoViewer->setCamera(camera);
}

void QuadViewerWidget::resizeChild(BaseViewer* viewer)
{
    bool isMaximized = viewer->IsMaximized();

    for (auto activeViewer : m_activeViewers)
    {
        if (activeViewer != viewer)
        {
            activeViewer->setVisible(!isMaximized);
        }
    }

    emit resizeWidget(this, isMaximized);
}

void QuadViewerWidget::updateAllDataBounds()
{
    // this method does the calculations for all data bounds
    // so each slice viewer only neeeds to be updated
    // { // set to default bounds dimension
    //     setAllDataBoundsToDefault();
    // }
    // { // expand bounds if bigger than dimension
    //     auto& allData = m_dataManager->getData();
    //     for (int i = 0; i < allData.size(); ++i)
    //     {
    //         // only 3d data
    //         if (!allData.at(i)->is3d())
    //             continue;
    //         auto data3d = (Data3d*)allData.at(i);
    //         m_allDataBounds[0] = std::min(m_allDataBounds[0],
    //             data3d->getBound(VisualAxis::X, true));
    //         m_allDataBounds[1] = std::max(m_allDataBounds[1],
    //             data3d->getBound(VisualAxis::X, false));
    //         m_allDataBounds[2] = std::min(m_allDataBounds[2],
    //             data3d->getBound(VisualAxis::Y, true));
    //         m_allDataBounds[3] = std::max(m_allDataBounds[3],
    //             data3d->getBound(VisualAxis::Y, false));
    //         m_allDataBounds[4] = std::min(m_allDataBounds[4],
    //             data3d->getBound(VisualAxis::Z, true));
    //         m_allDataBounds[5] = std::max(m_allDataBounds[5],
    //             data3d->getBound(VisualAxis::Z, false));
    //     }
    // }
    // Logger::trace(QString(
    //     "QuadViewerWidget -- updateAllDataBounds called bounds: %1, %2, %3, %4, %5, %6")
    //                   .arg(m_allDataBounds[0])
    //                   .arg(m_allDataBounds[1])
    //                   .arg(m_allDataBounds[2])
    //                   .arg(m_allDataBounds[3])
    //                   .arg(m_allDataBounds[4])
    //                   .arg(m_allDataBounds[5]));
    // // update the values in each viewer
    // for (int i = 0; i < m_sliceViewers.size(); ++i)
    //     m_sliceViewers[i]->setAllDataBounds(m_allDataBounds);
    // render();
}

void QuadViewerWidget::zoomIn()
{
    for (auto viewer : m_sliceViewers)
        viewer->zoomIn();
    // TODO: include stereo viewer in zoom functions
}

void QuadViewerWidget::zoomOut()
{
    for (auto viewer : m_sliceViewers)
        viewer->zoomOut();
    // TODO: include stereo viewer in zoom functions
}

void QuadViewerWidget::stereoZoomIn()
{
    for (auto viewer : m_stereoViewers)
        viewer->zoomIn();
}

void QuadViewerWidget::stereoZoomOut()
{
    for (auto viewer : m_stereoViewers)
        viewer->zoomOut();
}

QImage QuadViewerWidget::captureQuadViewerWidget()
{
    // allViewersList, allImagesList =
    // [[viewers/images in top splitter]
    //  [viewers/images in middle splitter]
    //  [viewers/images in bottom splitter]]
    std::vector<std::vector<QWidget*>> allViewersList;
    std::vector<std::vector<QImage>> allImagesList;
    std::vector<QSplitter*> allSplittersList;

    auto processSplitter =
        [&](QSplitter*
                splitter) { // check for visible viewers in each splitter and capture images of those widgets
            std::vector<QWidget*> viewersList;
            std::vector<QImage> imagesList;

            for (size_t i = 0; i < splitter->count(); ++i)
            {
                QWidget* viewer = splitter->widget(i);
                if (viewer->isVisible())
                {
                    viewersList.push_back(viewer);
                    QImage capture
                        = captureIndividualViewer(viewersList.back());
                    imagesList.push_back(capture);
                }
            }
            if (viewersList.size() > 0)
            {
                allViewersList.push_back(viewersList);
                allImagesList.push_back(imagesList);
                allSplittersList.push_back(splitter);
            }
        };

    processSplitter(m_topSplitter);
    processSplitter(m_middleSplitter);
    processSplitter(m_bottomSplitter);

    // Reconstruct quad viewer with QPainter
    // https://ultrast.atlassian.net/wiki/spaces/SD/pages/64552999/U6-1372+-+Save+3D+Viewer
    QImage quadViewerImage(this->size(), QImage::Format_RGB888);
    quadViewerImage.fill(Qt::black);
    QPainter painter(&quadViewerImage);
    int outlineThickness = 2;
    QPen pen(QColor(151, 151, 151), outlineThickness);
    painter.setPen(pen);

    if (allSplittersList.size() == 0)
        return quadViewerImage;

    int quadViewerOriginX = allViewersList.at(0).at(0)->pos().x();
    int quadViewerOriginY = allViewersList.at(0).at(0)->pos().y();
    QPoint quadViewerOrigin(quadViewerOriginX, quadViewerOriginY);

    for (size_t i = 0; i < allViewersList.size(); ++i)
    { // iterate through splitters
        QPoint splitterOrigin
            = allSplittersList.at(i)->pos() - quadViewerOrigin;
        for (size_t j = 0; j < allImagesList[i].size(); ++j)
        { // iterate within splitters
            QWidget* viewer = allViewersList.at(i).at(j);
            QImage image = allImagesList.at(i).at(j);

            QPoint viewerImageOffset((viewer->width() - image.width()) / 2,
                (viewer->height() - image.height()) / 2);
            QPoint imagePosition = viewer->pos() + splitterOrigin;
            QSize outlineOffset(outlineThickness * 2, outlineThickness * 2);
            QSize outlineSize = viewer->size() - outlineOffset;
            if (imagePosition.x() == 0)
            {
                imagePosition.setX(outlineThickness * 2);
                outlineSize.rwidth() -= outlineThickness * 2;
            }
            if (imagePosition.y() == 0)
            {
                imagePosition.setY(outlineThickness * 2);
                outlineSize.rheight() -= outlineThickness * 2;
            }
            QPoint imageInsertPosition = imagePosition + viewerImageOffset;
            painter.drawImage(imageInsertPosition, image);
            QRect outlineRect(imagePosition, outlineSize);
            painter.drawRect(outlineRect);
        }
    }

    return quadViewerImage;
}

QImage QuadViewerWidget::captureIndividualViewer(QWidget* viewer)
{
    auto currentViewer = qobject_cast<BaseViewer*>(viewer);
    QImage image = currentViewer->capture();
    return image;
}

void QuadViewerWidget::initializeUI()
{
    m_stereoViewer->hide();
    // m_flexibleSliceViewer->hide();
    m_mainSplitter->setChildrenCollapsible(false);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_mainSplitter);

    m_topSplitter->setChildrenCollapsible(false);
    m_middleSplitter->setChildrenCollapsible(false);
    m_bottomSplitter->setChildrenCollapsible(false);

    m_mainSplitter->addWidget(m_topSplitter);
    m_mainSplitter->addWidget(m_middleSplitter);
    m_mainSplitter->addWidget(m_bottomSplitter);
    setViewerLayout(ViewerLayout::DEFAULT);
    this->setLayout(layout);
}

void QuadViewerWidget::bindConnections()
{
    for (int i = 0; i < m_sliceViewers.size(); ++i)
    {
        // connect(m_sliceViewers[i], &SliceViewer::storeDistanceWidget,
        //     m_dataManager->getMeasurementManager(),
        //     &MeasurementManager3D::storeDistanceWidget);
        // connect(m_sliceViewers[i], &SliceViewer::storeAngleWidget,
        //     m_dataManager->getMeasurementManager(),
        //     &MeasurementManager3D::storeAngleWidget);
        connect(m_sliceViewers[i], &SliceViewer::zoomChanged, this,
            &QuadViewerWidget::zoomChanged);
    }

    connect(m_stereoViewer, &StereoViewer::zoomChanged, this,
        &QuadViewerWidget::zoomChanged);

    ////////////////////////3d measurement ////////////////////////////
    // m_3dVolumeMeasurer->setInteractor(m_axialViewer->getLabel(),
    //     m_axialViewer->getInteractor());
    // m_3dVolumeMeasurer->setInteractor(m_coronalViewer->getLabel(),
    //     m_coronalViewer->getInteractor());
    // m_3dVolumeMeasurer->setInteractor(m_sagittalViewer->getLabel(),
    //     m_sagittalViewer->getInteractor());

    // m_3dVolumeMeasurer->setRenderer(m_axialViewer->getLabel(),
    //     m_axialViewer->getRenderer());
    // m_3dVolumeMeasurer->setRenderer(m_coronalViewer->getLabel(),
    //     m_coronalViewer->getRenderer());
    // m_3dVolumeMeasurer->setRenderer(m_sagittalViewer->getLabel(),
    //     m_sagittalViewer->getRenderer());

    connect(m_axialViewer, &SliceViewer::mouseMove, this,
        &QuadViewerWidget::mouseMove);
    connect(m_coronalViewer, &SliceViewer::mouseMove, this,
        &QuadViewerWidget::mouseMove);
    connect(m_sagittalViewer, &SliceViewer::mouseMove, this,
        &QuadViewerWidget::mouseMove);
    connect(m_axialViewer, &SliceViewer::leftClick, this,
        &QuadViewerWidget::leftClick);
    connect(m_coronalViewer, &SliceViewer::leftClick, this,
        &QuadViewerWidget::leftClick);
    connect(m_sagittalViewer, &SliceViewer::leftClick, this,
        &QuadViewerWidget::leftClick);
    ///////////////////////////////////////////////////////////////////

    connect(m_axialViewer, &SliceViewer::resizeSplitter, this,
        &QuadViewerWidget::resizeChild);
    connect(m_coronalViewer, &SliceViewer::resizeSplitter, this,
        &QuadViewerWidget::resizeChild);
    connect(m_sagittalViewer, &SliceViewer::resizeSplitter, this,
        &QuadViewerWidget::resizeChild);
    connect(m_stereoViewer, &SliceViewer::resizeSplitter, this,
        &QuadViewerWidget::resizeChild);
    connect(m_stereoViewer, &StereoViewer::resizeButtonClicked, this,
        &QuadViewerWidget::stereoViewerResizeClicked);
    // connect(m_flexibleSliceViewer, &FlexibleSliceViewer::resizeSplitter, this,
    //     &QuadViewerWidget::resizeChild);

    connect(m_viewerManager, &ViewerManager::buttonModifyOrigin, this,
        &QuadViewerWidget::buttonModifyOrigin);
    connect(m_viewerManager, &ViewerManager::buttonModifyOrientation, this,
        &QuadViewerWidget::buttonModifyOrientation);
    connect(m_viewerManager, &ViewerManager::buttonModifySize, this,
        &QuadViewerWidget::buttonModifySize);

    connect(m_viewerManager, &ViewerManager::leftButtonDown, this,
        &QuadViewerWidget::leftButtonDown);
    connect(m_viewerManager, &ViewerManager::middleButtonDown, this,
        &QuadViewerWidget::middleButtonDown);
    connect(m_viewerManager, &ViewerManager::middleButtonUp, this,
        &QuadViewerWidget::middleButtonUp);
    connect(m_viewerManager, &ViewerManager::rightButtonDown, this,
        &QuadViewerWidget::rightButtonDown);

    connect(m_viewerManager, &ViewerManager::coordinateChanged, this,
        &QuadViewerWidget::coordinateChanged);

    // connect(this, &QuadViewerWidget::robotPoseUpdate, m_flexibleSliceViewer,
    //     &FlexibleSliceViewer::poseUpdated);
}

// void QuadViewerWidget::setAllDataBoundsToDefault()
// {
//     double defaultPosition[3], defaultCrosshairLength[3];
//     defaultPosition[0]
//         = Setting::getSystemSetting().getDouble("viewer.defaultPositionX");
//     defaultPosition[1]
//         = Setting::getSystemSetting().getDouble("viewer.defaultPositionY");
//     defaultPosition[2]
//         = Setting::getSystemSetting().getDouble("viewer.defaultPositionZ");
//     defaultCrosshairLength[0] = Setting::getSystemSetting().getDouble(
//         "viewer.defaultCrosshairLengthX");
//     defaultCrosshairLength[1] = Setting::getSystemSetting().getDouble(
//         "viewer.defaultCrosshairLengthY");
//     defaultCrosshairLength[2] = Setting::getSystemSetting().getDouble(
//         "viewer.defaultCrosshairLengthZ");

//     // set to default bounds
//     m_allDataBounds[0] = defaultPosition[0] - (defaultCrosshairLength[0] / 2.0);
//     m_allDataBounds[1] = defaultPosition[0] + (defaultCrosshairLength[0] / 2.0);
//     m_allDataBounds[2] = defaultPosition[1] - (defaultCrosshairLength[1] / 2.0);
//     m_allDataBounds[3] = defaultPosition[1] + (defaultCrosshairLength[1] / 2.0);
//     m_allDataBounds[4] = defaultPosition[2] - (defaultCrosshairLength[2] / 2.0);
//     m_allDataBounds[5] = defaultPosition[2] + (defaultCrosshairLength[2] / 2.0);
// }

void QuadViewerWidget::setAllDataBoundsToSliceViewers()
{
    for (int i = 0; i < m_sliceViewers.size(); ++i)
    {
        m_sliceViewers[i]->setAllDataBounds(m_allDataBounds);
    }
}

void QuadViewerWidget::setViewerLayout(const ViewerLayout& layout)
{
    int screenWidth = QGuiApplication::primaryScreen()->virtualSize().width();
    int screenHeight = QGuiApplication::primaryScreen()->virtualSize().height();
    // int topHeight = screenHeight * 2 / 3;
    // int lowHeight = screenHeight - topHeight;
    int topHeight = screenHeight / 2;
    int lowHeight = screenHeight / 2;
    if (layout != ViewerLayout::REGISTRATION)
    {
        m_stereoViewer->show();
        emit stereoViewerVisible(true);
    }
    switch (layout)
    {
    case ViewerLayout::BIOPSY_PLAN:
    {
        if (m_screenOrientation == Qt::Orientation::Horizontal)
        {
            m_topSplitter->insertWidget(0, m_axialViewer);
            m_topSplitter->insertWidget(1, m_stereoViewer);
            m_bottomSplitter->insertWidget(0, m_coronalViewer);
            m_bottomSplitter->insertWidget(1, m_sagittalViewer);
            // m_bottomSplitter->insertWidget(2, m_flexibleSliceViewer);
            m_middleSplitter->hide();
            setSplitterSize(*m_topSplitter, screenWidth);
            setSplitterSize(*m_bottomSplitter, screenWidth);
            QList<int> mainSplitterSizes = { topHeight, 0, lowHeight };
            m_mainSplitter->setSizes(mainSplitterSizes);
        }
        else
        {
            m_topSplitter->insertWidget(0, m_axialViewer);
            m_middleSplitter->insertWidget(0, m_coronalViewer);
            m_middleSplitter->insertWidget(1, m_sagittalViewer);
            m_bottomSplitter->insertWidget(0, m_stereoViewer);
            // m_bottomSplitter->insertWidget(1, m_flexibleSliceViewer);
            m_middleSplitter->show();
            setSplitterSize(*m_middleSplitter, screenHeight);
            setSplitterSize(*m_bottomSplitter, screenHeight);
            QList<int> mainSplitterSizes
                = { topHeight / 2, lowHeight / 2, screenHeight / 3 };
            m_mainSplitter->setSizes(mainSplitterSizes);
        }
        break;
    }

    case ViewerLayout::REGISTRATION:
    {
        m_topSplitter->insertWidget(0, m_axialViewer);
        m_topSplitter->insertWidget(1, m_stereoViewer);
        m_stereoViewer->hide();
        emit stereoViewerVisible(false);
        m_bottomSplitter->insertWidget(0, m_coronalViewer);
        m_bottomSplitter->insertWidget(1, m_sagittalViewer);
        // m_bottomSplitter->insertWidget(2, m_flexibleSliceViewer);
        m_middleSplitter->hide();
        setSplitterSize(*m_topSplitter, screenWidth);
        setSplitterSize(*m_bottomSplitter, screenWidth);
        setSplitterSize(*m_mainSplitter, screenHeight);
        QList<int> mainSplitterSizes = { topHeight, 0, lowHeight };
        m_mainSplitter->setSizes(mainSplitterSizes);
        break;
    }

    case ViewerLayout::BIOPSY_OPERATION:
    {
        m_topSplitter->insertWidget(0, m_axialViewer);
        m_topSplitter->insertWidget(1, m_stereoViewer);
        m_bottomSplitter->insertWidget(0, m_coronalViewer);
        m_bottomSplitter->insertWidget(1, m_sagittalViewer);
        m_middleSplitter->hide();
        setSplitterSize(*m_topSplitter, screenWidth);
        setSplitterSize(*m_bottomSplitter, screenWidth);
        QList<int> mainSplitterSizes = { topHeight, 0, lowHeight };
        m_mainSplitter->setSizes(mainSplitterSizes);
        break;
    }

    default:
    {
        m_topSplitter->insertWidget(0, m_axialViewer);
        m_topSplitter->insertWidget(1, m_stereoViewer);
        m_bottomSplitter->insertWidget(0, m_coronalViewer);
        m_bottomSplitter->insertWidget(1, m_sagittalViewer);
        // m_bottomSplitter->insertWidget(2, m_flexibleSliceViewer);
        m_middleSplitter->hide();
        setSplitterSize(*m_topSplitter, screenWidth);
        setSplitterSize(*m_bottomSplitter, screenWidth);
        QList<int> mainSplitterSizes = { topHeight, 0, lowHeight };
        m_mainSplitter->setSizes(mainSplitterSizes);
        break;
    }
    }
}

void QuadViewerWidget::setSplitterSize(QSplitter& splitter,
    const int maxSizeValue)
{
    QList<int> splitterSizeList;
    for (int i = 0; i < splitter.count(); ++i)
        splitterSizeList.push_back(maxSizeValue);
    splitter.setSizes(splitterSizeList);
}

void QuadViewerWidget::updateScreenOrientation(
    const Qt::Orientation orientation)
{
    m_screenOrientation = orientation;
}

void QuadViewerWidget::leftClick(QString viewerLabel, const unit::Point& coord)
{
    // m_3dVolumeMeasurer->clickToAddPoint(viewerLabel, coord);
}

void QuadViewerWidget::mouseMove(QString viewerLabel, const unit::Point& coord)
{
    // m_3dVolumeMeasurer->moveToGenerateEllipsoid(viewerLabel, coord);
}

// void QuadViewerWidget::deleteVolumeWidgets(bool isAll,
//     MeasurementType measurementType, int subIndex)
// {
// if (isAll)
// {
//     m_3dVolumeMeasurer->deleteAllVolumes();
// }
// else
// {
//     if (measurementType == MeasurementType::VOLUME)
//     {
//         m_3dVolumeMeasurer->deleteOneVolume(subIndex);
//     }
// }
// }

// void QuadViewerWidget::reviewMaximizeStereoViewer(const bool& toBeMaximized)
// {
//     if (toBeMaximized)
//     {
//         m_stereoViewerOldViewAngle = m_stereoViewer->getViewAngle();
//         m_stereoViewerOldVisibility = m_stereoViewer->isVisible();
//         if (!m_stereoViewerOldVisibility)
//             m_stereoViewer->show();
//         // if (!m_stereoViewer->IsMaximized())
//         //     m_stereoViewer->resizeSelf();
//         m_stereoViewer->setViewAngle(STEREOVIEWER_DEFAULT_VIEW_ANGLE / 4);
//         //ToDo:: remove this after the side by side view is fixed
//         m_stereoViewer->setVtkResizeButtonEnable(m_stereoViewer->getRenderer(),
//             false);
//     }
//     else
//     {
//         // if (m_stereoViewer->IsMaximized())
//         //     m_stereoViewer->resizeSelf();
//         if (!m_stereoViewerOldVisibility)
//             m_stereoViewer->hide();
//         m_stereoViewer->setViewAngle(m_stereoViewerOldViewAngle);
//         m_stereoViewer->setVtkResizeButtonEnable(m_stereoViewer->getRenderer(),
//             true);
//     }
// }
}
}
