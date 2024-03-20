#pragma once
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

#include <BaseData.h>
#include <VisualizationDataType.h>
#include <unit.h>

class vtkPlane;
class vtkBox;

namespace ultrast {
namespace infrastructure {
namespace utility {
using VisualAxis = ultrast::infrastructure::utility::VisualAxis;
using ViewerType = ultrast::infrastructure::utility::ViewerType;
using ViewerInfo = ultrast::infrastructure::utility::ViewerInfo;

class Coordinate;
class CoordinateRepository;

class Data3d : public BaseData
{
    Q_OBJECT
public:
    Data3d(int id = -1, QString name = QString(), bool hasFilePath = false);
    void onDelete(const bool& modify = true) override;
    virtual ~Data3d();
    void setUpForViewer(ViewerInfo info);
    void setUpForViewer(const QString& viewerId,
        const FlexibleSliceDataInfo& info);
    void resetTransform();
    //get methods
    virtual void getBounds(double (&bounds)[6]) = 0;
    const double getBound(VisualAxis axis, bool min);
    vtkSmartPointer<vtkTransform> getDataTransform() const
    {
        return m_transform;
    };
    vtkSmartPointer<vtkTransform> getWorldTransform() const
    {
        return m_worldTransform;
    };
    void getDataTranslation(double (&translation)[3]) const;
    void getWorldTranslation(double (&translation)[3]) const;
    const unit::Point getDataPosition() const;
    const unit::Point getWorldPosition() const;
    void getDataOrientation(double (&orientation)[3]) const;
    void getWorldOrientation(double (&orientation)[3]) const;
    void getDataScale(double (&scale)[3]) const;
    void getWorldScale(double (&scale)[3]) const;
    /**
    * @brief get the rotation center in local coordinate system
    */
    const unit::Point& getDataRotationCenter();
    /**
    * @brief get the rotation center in world coordinate system
    */
    const unit::Point getWorldRotationCenter();
    const unit::Point getLocalRotationCenter();
    const std::string getCoordinateId() const { return m_coordinateId; };
    Coordinate* getCoordinate() const;
    /**
    * @brief adds a BaseData subclass as a child and updates the child world transform matrix
    *
    * Will override the parent if the child already has a parent
    *
    * @param child The BaseData subclass which can't be itself or the root parent
    * @param modify Will modify the transform to maintain the world position, set to false to retain m_transform
    * @return bool of the result of the operation
    */
    // const bool addChild(BaseData* child, const bool& modify = true) override;
    /**
    * @brief removes a BaseData subclass from m_children and updates the child world transform matrix
    *
    * Will do nothing if the child does not exist
    *
    * @param child The BaseData subclass which is a child of the current data
    * @param modify Will modify the transform to maintain the world position, set to false to retain m_transform
    * @return bool of the result of the operation
    */
    // const bool removeChild(BaseData* child, const bool& modify = true) override;
signals:
    /**
    * @brief only need to emit this if position does not change
    *
    * boundsUpdated is called automatically everytime positionUpdated is emitted
    */
    void boundsUpdated();
    void positionUpdated();
    void render();
public slots:
    void concatenateDataTransform(unit::TransformMatrix transform);
    void concatenateDataTransform(vtkSmartPointer<vtkTransform> transform);
    void setDataTransform(unit::TransformMatrix transform);
    void setDataTransform(vtkSmartPointer<vtkTransform> transform);
    void multiplyDataTranform(unit::TransformMatrix transform);
    /** add a translation to existing transform
    *
    * concatenate another xyz translation onto the current m_transform
    *
    * @param x world position of x coord
    * @param y world position of y coord
    * @param z world position of z coord
    */
    void setDataTranslation(double x, double y, double z);
    /** sets the current translation transformation to the position
    *
    * Resets the matrix to identity and sets the traslation to the position values
    *
    * @param coord size 3 array for x, y, z coord
    */
    void setDataPosition(const unit::Point& coord);
    /** add a rotation to existing transform
    *
    * concatenate another xyz rotation onto the current m_transform in the order of x, y, z
    *
    * @param x world position of x coord
    * @param y world position of y coord
    * @param z world position of z coord
    */
    void setDataRotation(double x, double y, double z);
    /** add a scale to exisisting transform
    *
    * concatenate another xyz scale onto the current m_transform
    *
    * @param x world position of x coord
    * @param y world position of y coord
    * @param z world position of z coord
    */
    void setDataScale(double x, double y, double z);

    /** set the new world transform to the data and change its local transform w.r.t its parent
    *
    * @param worldTransform the new world transform to apply
    */
    void setWorldTransform(const vtkSmartPointer<vtkTransform> worldTransform);

    /** set the new world transform to the data and change its local transform w.r.t its parent
    *
    * @param worldTransform the new world transform to apply
    */
    void setWorldPosition(const unit::Point& coord);

    /**
    * @brief called when Coordinate->setTransform() is called to update m_worldTransform
    */
    void updateWorldTransform();
    /**
    * @brief set the rotation center in local coordinate system
    */
    void setDataRotationCenter(unit::Point point);
    /**
    * @brief set the rotation center in the world coordinate system
    */
    void setWorldRotationCenter(unit::Point point);

protected:
    virtual void setUpForSliceViewer(ViewerInfo info) {};
    virtual void setUpForStereoViewer(ViewerInfo info) {};
    virtual void setUpFlexibleSliceViewer(const QString& viewerId,
        const FlexibleSliceDataInfo& info)
    {
    }
    /**
    * @brief get a QJsonObject with the values of the BaseData variables
    *
    * Gets the variables for m_dataName, m_isVisible, m_transform, and all qProperties in the QJsonObject
    *
    * @return QJsonObject
    */
    QJsonObject getBaseObject() const override;
    /**
    * @brief set the BaseData variables with a QJsonObject
    *
    * Sets the variables for m_dataName, m_isVisible, m_transform, and all qProperties from the QJsonObject
    *
    * @param const QJsonObject&
    */
    void setBaseObject(const QJsonObject& object) override;

    //need to be used in subclass to link to a transformFilter or equivalent, is the transform to world Coordinate
    vtkSmartPointer<vtkTransform> m_worldTransform;
    vtkSmartPointer<vtkActor> m_boundsActor;

    template <typename T>
    bool existsInMap(std::map<QString, T> map, QString key) const
    {
        return map.find(key) != map.end();
    }

private:
    CoordinateRepository* m_coordinateRepository;
    //just used for storage
    //m_transform represents the transform to the parent Coordinate system
    vtkSmartPointer<vtkTransform> m_transform;
    std::string m_coordinateId;
    //this point should be in the local data coordinate system, else if the transform changes, it doesn't move with it
    unit::Point m_rotationCenter;
};

} // namespace utility
} // namespace interface
}; // namespace ultrast
