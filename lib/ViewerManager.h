#pragma once

// #include <domain\data\DataManager.h>
#include <QObject>
#include <VisualizationDataType.h>
#include <unit.h>
#include <vector>

namespace ultrast {
namespace infrastructure {
namespace utility {
class Data3d;
}
}
}

using Data3d = ultrast::infrastructure::utility::Data3d;
// using DataManager = ultrast::domain::data::DataManager;
using VisualAxis = ultrast::infrastructure::utility::VisualAxis;
namespace unit = ultrast::infrastructure::utility::unit;

// namespace ultrast {
// namespace domain {
// namespace data {
//     class DataManager;
// }
// }
// }

namespace unit = ultrast::infrastructure::utility::unit;

namespace ultrast {
namespace visualization {

enum ViewerWindowType : unsigned int;
enum ButtonAxisSubType : unsigned int;

class BaseViewer;

class ViewerManager : public QObject
{
    Q_OBJECT
public:
    ViewerManager(QObject* parent);
    ~ViewerManager();
    void registerViewer(BaseViewer* viewer);
    void setCrosshairCoordinate(const unit::Point& coord);
    const unit::Point getCrosshairCoordinate();
    void resetCamera();
    void resetCamera(double bounds[6]);
    void render();
    // void filterOutVolumeImageById(const std::vector<int>& idList);
    // void resetVolumeImageFilters();
    void addData(Data3d& data);
    void removeData(Data3d& data);
signals:
    void coordinateChanged(const unit::Point& coord);
    void leftButtonDown(const unit::Point& coord);
    void middleButtonDown(const unit::Point& coord);
    void middleButtonUp(const unit::Point& coord);
    void rightButtonDown(const unit::Point& coord);
    void modifySurgeryModelOrigin(ViewerWindowType viewerType,
        ButtonAxisSubType subType);
    void resizeSplitter(BaseViewer* viewer);
    void buttonModifyOrigin(VisualAxis axis, bool increase);
    void buttonModifyOrientation(VisualAxis axis, bool increase);
    void buttonModifySize(VisualAxis axis, bool increase);
public slots:
    void coordinateChangedHandler(const unit::Point& coord);

private:
    void setPreviousCoordinateToDefault();

    std::vector<BaseViewer*> m_viewers;
    unit::Point m_previousCoordinate;
    // DataManager* m_dataManager;
    std::vector<int> m_filteredVolumeImageIdList;
};
}
}
