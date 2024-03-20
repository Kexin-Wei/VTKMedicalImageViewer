#include "ViewerManager.h"
#include "BaseOverlayButton.h"
#include "BaseViewer.h"
#include "SliceViewer.h"
// #include "domain\data\DataManager.h"
// #include <infrastructure\utility\Setting.h>
// #include <infrastructure\utility\Logger.h>
#include <Data3d.h>
#include <vtkDataSetMapper.h>

// using Setting = ultrast::infrastructure::utility::Setting;
// using Logger = ultrast::infrastructure::utility::Logger;
using Data3d = ultrast::infrastructure::utility::Data3d;

namespace ultrast {
namespace visualization {

ViewerManager::ViewerManager(QObject* parent) :
    QObject(parent)
// m_dataManager(DataManager::getInstance())
{
    // this->setPreviousCoordinateToDefault();
    // connect(this, &ViewerManager::coordinateChanged,
    //     m_dataManager->getMeasurementManager(),
    //     &MeasurementManager3D::checkMeasurementVisibility);
}

ViewerManager::~ViewerManager()
{
}

void ViewerManager::registerViewer(BaseViewer* viewer)
{
    //check if already exists
    if (std::find(m_viewers.begin(), m_viewers.end(), viewer)
        != m_viewers.end())
    {
        return;
    }

    viewer->bindViewerManager(this);
    m_viewers.push_back(viewer);
}

void ViewerManager::setCrosshairCoordinate(const unit::Point& coord)
{
    m_previousCoordinate = coord;
}

const unit::Point ViewerManager::getCrosshairCoordinate()
{
    return m_previousCoordinate;
}

void ViewerManager::setFocalPoint(const unit::Point& coord)
{
    for (auto& baseViewer : m_viewers)
    {
        baseViewer->setFocalPoint(coord);
    }
}

void ViewerManager::setAllDataBounds(double* bounds)
{
    for (auto& baseViewer : m_viewers)
    {
        if (baseViewer->getType() == ViewerType::SLICE)
        {
            ((SliceViewer*)baseViewer)->setAllDataBounds(bounds);
        }
    }
}

void ViewerManager::resetCamera()
{
    for (auto& baseViewer : m_viewers)
    {
        baseViewer->resetCamera();
    }
    // setPreviousCoordinateToDefault();
}

void ViewerManager::resetCamera(double bounds[6])
{
    for (auto& baseViewer : m_viewers)
    {
        baseViewer->resetCamera(bounds);
    }
}

void ViewerManager::render()
{
    for (auto it = m_viewers.begin(); it != m_viewers.end(); ++it)
        (*it)->render();
}

// void ViewerManager::filterOutVolumeImageById(const std::vector<int>& idList)
// {
//     m_filteredVolumeImageIdList = idList;
//     for (int i = 0; i < idList.size(); ++i)
//     {
//         Data3d* data = m_dataManager->getDataByIdIf3d(idList[i]);
//         for (int j = 0; j < m_viewers.size(); ++j)
//         {
//             m_viewers[j]->removeData(*data);
//         }
//     }
// }

// void ViewerManager::resetVolumeImageFilters()
// {
//     for (int i = 0; i < m_filteredVolumeImageIdList.size(); ++i)
//     {
//         Data3d* data
//             = m_dataManager->getDataByIdIf3d(m_filteredVolumeImageIdList[i]);
//         if (data)
//         {
//             for (int j = 0; j < m_viewers.size(); ++j)
//             {
//                 m_viewers[j]->addData(*data);
//             }
//         }
//     }
//     m_filteredVolumeImageIdList.clear();
// }

void ViewerManager::addData(Data3d& data)
{
    for (size_t i = 0; i < m_viewers.size(); ++i)
    {
        m_viewers[i]->addData(data);
    }
}

void ViewerManager::removeData(Data3d& data)
{
    for (size_t i = 0; i < m_viewers.size(); ++i)
    {
        m_viewers[i]->removeData(data);
    }
}

void ViewerManager::coordinateChangedHandler(const unit::Point& coord)
{
    m_previousCoordinate = coord;
    emit coordinateChanged(coord);
}

// void ViewerManager::setPreviousCoordinateToDefault()
// {
//     double defaultPosition[3];
//     defaultPosition[0]
//         = Setting::getSystemSetting().getDouble("viewer.defaultPositionX");
//     defaultPosition[1]
//         = Setting::getSystemSetting().getDouble("viewer.defaultPositionY");
//     defaultPosition[2]
//         = Setting::getSystemSetting().getDouble("viewer.defaultPositionZ");
//     m_previousCoordinate = defaultPosition;
// }

}
}
