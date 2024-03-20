#include <Coordinate.h>
#include <CoordinateRepository.h>
#include <Data3d.h>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <vtkBox.h>
#include <vtkPlane.h>

namespace ultrast {
namespace infrastructure {
namespace utility {

Data3d::Data3d(int id, QString name, bool hasFilePath) :
    BaseData(id, name, hasFilePath, true),
    m_transform(vtkSmartPointer<vtkTransform>::New()),
    m_worldTransform(vtkSmartPointer<vtkTransform>::New()),
    m_boundsActor(vtkSmartPointer<vtkActor>::New()),
    m_coordinateId(""),
    m_rotationCenter(0, 0, 0),
    m_coordinateRepository(CoordinateRepository::getInstance())
{ // if for some reason it is possible to generate data faster than a millisecond
    Coordinate* coordinate = nullptr;
    std::string idString = "";
    for (int i = 0; i < 50 && !coordinate; i++)
    { // loop if nullptr with hard limit of 50 iterations
        // generate data id with time in future since current already exists
        idString = QString("data%1")
                       .arg(QDateTime::currentMSecsSinceEpoch() + i)
                       .toStdString();
        coordinate
            = m_coordinateRepository->createCoordinate(idString, "world");
    }
    if (!coordinate)
        qDebug() << "Data3d -- Failed to create unique coordinate repository";
    else
    {
        m_coordinateId = idString;
        connect(coordinate, &Coordinate::transformUpdated, this,
            &Data3d::updateWorldTransform);
    }
    // every time positions change, bounds must be updated too
    connect(this, &Data3d::positionUpdated, this, &Data3d::boundsUpdated);
}

void Data3d::onDelete(const bool& modify)
{
    qDebug() << "Data3d -- onDelete called with modify = "
             << (modify ? "true" : "false");
    if (modify)
    {
        // transform the matrix of child to word coordinate
        // for (auto child : m_children)
        //     child->setDataTransform(m_coordinateRepository->getTransform(child->getCoordinateId(), "world"));
        BaseData::onDelete(modify);
        qDebug() << "Data3d -- Deleting coordinate from repository";
        m_coordinateRepository->deleteCoordinate(getCoordinateId());
        qDebug() << "Data3d -- Finished deleting coordinate from repository";
        // for (auto data : get3dChildren())
        //     ((Data3d*)data)->updateWorldTransform();
    }
}

Data3d::~Data3d()
{
}

void Data3d::setUpForViewer(ViewerInfo info)
{
    switch (info.m_type)
    {
    case ViewerType::SLICE:
        setUpForSliceViewer(info);
        break;
    case ViewerType::STEREO:
        setUpForStereoViewer(info);
        break;
    }
}

void Data3d::setUpForViewer(const QString& viewerId,
    const FlexibleSliceDataInfo& info)
{
    setUpFlexibleSliceViewer(viewerId, info);
}

void Data3d::resetTransform()
{
    m_transform->Identity();
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
}

const double Data3d::getBound(VisualAxis axis, bool min)
{
    double bounds[6];
    getBounds(bounds);
    switch (axis)
    {
    case VisualAxis::X:
        return (min ? bounds[0] : bounds[1]);
    case VisualAxis::Y:
        return (min ? bounds[2] : bounds[3]);
    case VisualAxis::Z:
        return (min ? bounds[4] : bounds[5]);
    }
}

void Data3d::getDataTranslation(double (&translation)[3]) const
{
    m_transform->GetPosition(translation);
}

void Data3d::getWorldTranslation(double (&translation)[3]) const
{
    m_worldTransform->GetPosition(translation);
}

const unit::Point Data3d::getDataPosition() const
{
    double coordArray[3];
    m_transform->GetPosition(coordArray);
    return unit::Point(coordArray);
}

const unit::Point Data3d::getWorldPosition() const
{
    double coordArray[3];
    m_worldTransform->GetPosition(coordArray);
    return unit::Point(coordArray);
}

void Data3d::getDataOrientation(double (&orientation)[3]) const
{
    m_transform->GetOrientation(orientation);
}

void Data3d::getWorldOrientation(double (&orientation)[3]) const
{
    m_worldTransform->GetOrientation(orientation);
}

void Data3d::getDataScale(double (&scale)[3]) const
{
    m_transform->Scale(scale);
}

void Data3d::getWorldScale(double (&scale)[3]) const
{
    m_worldTransform->Scale(scale);
}

Coordinate* Data3d::getCoordinate() const
{
    if (!getCoordinateId().empty())
        return m_coordinateRepository->getCoordinate(getCoordinateId());
    return nullptr;
}

// const bool Data3d::addChild(BaseData* child, const bool& modify)
// {
//     if (BaseData::addChild(child, modify))
//     {
//         if (child->is3d())
//         {
//             Logger::trace(
//                 QString("Data3d -- add child 3d function called from (id:%1).")
//                     .arg(getId()));
//             auto child3d = (Data3d*)child;
//             m_coordinateRepository->setCoordinateParent(
//                 child3d->getCoordinateId(), getCoordinateId());

//             if (modify) // modify transform to maintain world coordinates
//             {
//                 // a lambda function to print vtkTransform for trouble shooting
//                 auto printTf = [](vtkSmartPointer<vtkTransform> tf) {
//                     auto tfMatrix = unit::TransformMatrix::New();
//                     tf->GetMatrix(tfMatrix);
//                     for (unsigned int i = 0; i < 4; i++)
//                         Logger::trace(QString("| %1, %2, %3, %4 |")
//                                           .arg(tfMatrix->GetElement(i, 0))
//                                           .arg(tfMatrix->GetElement(i, 1))
//                                           .arg(tfMatrix->GetElement(i, 2))
//                                           .arg(tfMatrix->GetElement(i, 3)));
//                     Logger::trace("-------------------------------");
//                 };

//                 auto childWorld = child3d->getWorldTransform();
//                 printTf(childWorld);
//                 auto parentWorldInvert = m_worldTransform;
//                 parentWorldInvert->Inverse();
//                 printTf(parentWorldInvert);
//                 vtkSmartPointer<vtkTransform> modifyMatrix
//                     = vtkSmartPointer<vtkTransform>::New();
//                 modifyMatrix->Concatenate(parentWorldInvert);
//                 modifyMatrix->Concatenate(childWorld);
//                 modifyMatrix->Update();
//                 printTf(modifyMatrix);
//                 child3d->setDataTransform(
//                     modifyMatrix); // new m_transform for child data so that it can maintain world transform
//             }
//             updateWorldTransform();
//         }
//         return true;
//     }
//     return false;
// }

// const bool Data3d::removeChild(BaseData* child, const bool& modify)
// {
//     Logger::trace(
//         QString("Data3d -- remove child (id:%1) function called from (id:%2).")
//             .arg(child->getId())
//             .arg(getId()));
//     if (BaseData::removeChild(child, modify))
//     {
//         if (child->is3d())
//         {
//             auto child3d = (Data3d*)child;
//             auto childWorld = child3d->getWorldTransform();
//             if (!childWorld)
//             {
//                 Logger::debug(QString(
//                     "Data3d -- childWorld is a null pointer. Operation will not proceed further."));
//                 return false;
//             }
//             m_coordinateRepository->setCoordinateParent(
//                 child3d->getCoordinateId(), "world");
//             child3d->setDataTransform(childWorld);
//             updateWorldTransform();
//         }
//         return true;
//     }
//     return false;
// }

void Data3d::concatenateDataTransform(unit::TransformMatrix transform)
{
    m_transform->Concatenate(transform);
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
}

void Data3d::concatenateDataTransform(vtkSmartPointer<vtkTransform> transform)
{
    m_transform->Concatenate(transform->GetMatrix());
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
}

void Data3d::setDataTransform(unit::TransformMatrix transform)
{
    m_transform->SetMatrix(transform);
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
}

void Data3d::setDataTransform(vtkSmartPointer<vtkTransform> transform)
{
    m_transform->SetMatrix(transform->GetMatrix());
    m_transform->Update();

    auto coordinate = getCoordinate();
    if (coordinate)
    {
        coordinate->setTransform(m_transform->GetMatrix());
    }
}

void Data3d::multiplyDataTranform(unit::TransformMatrix transform)
{
    unit::TransformMatrix result = unit::TransformMatrix::New();
    vtkMatrix4x4::Multiply4x4(m_transform->GetMatrix(), transform, result);
    setDataTransform(result);
}

void Data3d::setDataTranslation(double x, double y, double z)
{
    m_transform->Translate(x, y, z);
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
}

void Data3d::setDataPosition(const unit::Point& coord)
{
    double coordArray[3];
    coord.toArray(coordArray);
    auto data = m_transform->GetMatrix()->GetData();
    data[3] = coordArray[0];
    data[7] = coordArray[1];
    data[11] = coordArray[2];
    m_transform->SetMatrix(data);
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
}

void Data3d::setDataRotation(double x, double y, double z)
{
    m_transform->Translate(m_rotationCenter[0], m_rotationCenter[1],
        m_rotationCenter[2]);
    m_transform->RotateWXYZ(x, 1, 0, 0);
    m_transform->RotateWXYZ(y, 0, 1, 0);
    m_transform->RotateWXYZ(z, 0, 0, 1);
    m_transform->Translate(-m_rotationCenter[0], -m_rotationCenter[1],
        -m_rotationCenter[2]);
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
}

void Data3d::setDataScale(double x, double y, double z)
{
    m_transform->Scale(x, y, z);
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
}

void Data3d::setWorldTransform(
    const vtkSmartPointer<vtkTransform> worldTransform)
{
    std::string parentCoordinateId
        = m_coordinateRepository->getParentCoodinateId(getCoordinateId());
    if (parentCoordinateId == "world") // world is parent Coordinate
    {
        setDataTransform(worldTransform);
    }
    else
    {
        auto parentTransformMatrix
            = m_coordinateRepository->getParentWorldTransform(
                getCoordinateId());
        vtkSmartPointer<vtkTransform> parentTransform
            = vtkSmartPointer<vtkTransform>::New();
        parentTransform->SetMatrix(parentTransformMatrix);
        parentTransform->Inverse();
        vtkSmartPointer<vtkTransform> newDataTransform
            = vtkSmartPointer<vtkTransform>::New();
        newDataTransform->Concatenate(parentTransform);
        newDataTransform->Concatenate(worldTransform);
        newDataTransform->Update();
        setDataTransform(newDataTransform);
    }
}

void Data3d::setWorldPosition(const unit::Point& coord)
{
    double coordArray[3];
    vtkSmartPointer<vtkTransform> newWorldTransform
        = vtkSmartPointer<vtkTransform>::New(); // forming a new world transform
    coord.toArray(coordArray);
    auto data = m_worldTransform->GetMatrix()->GetData();
    data[3] = coordArray[0];
    data[7] = coordArray[1];
    data[11] = coordArray[2];
    newWorldTransform->SetMatrix(data);
    newWorldTransform->Update();
    setWorldTransform(newWorldTransform);
}

void Data3d::updateWorldTransform()
{
    auto matrix
        = m_coordinateRepository->getTransform("world", getCoordinateId());
    m_worldTransform->SetMatrix(matrix);
    m_worldTransform->Update();
    emit positionUpdated();
    for (auto child : m_children)
        if (child->is3d())
            ((Data3d*)child)->updateWorldTransform();
}

void Data3d::setDataRotationCenter(unit::Point point)
{
    m_rotationCenter = point;
}

void Data3d::setWorldRotationCenter(unit::Point point)
{
    if (m_rotationCenter != point)
    {
        double localRotationCenter[3] = { point.x, point.y, point.z };
        m_worldTransform->GetInverse()->TransformPoint(localRotationCenter,
            localRotationCenter);
        m_rotationCenter = localRotationCenter;
    }
}

const unit::Point& Data3d::getDataRotationCenter()
{
    return m_rotationCenter;
}

const unit::Point Data3d::getWorldRotationCenter()
{
    double worldRotationCenter[3]
        = { m_rotationCenter.x, m_rotationCenter.y, m_rotationCenter.z };
    m_worldTransform->TransformPoint(worldRotationCenter, worldRotationCenter);
    return worldRotationCenter;
}

const unit::Point Data3d::getLocalRotationCenter()
{
    return m_rotationCenter;
}

QJsonObject Data3d::getBaseObject() const
{
    auto object = BaseData::getBaseObject();
    QJsonArray transform;
    // convert 4x4 matrix to array
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            transform.push_back(m_transform->GetMatrix()->GetElement(i, j));
    object["transform"] = transform;
    QJsonArray rotationCenter;
    for (int i = 0; i < m_rotationCenter.size(); ++i)
        rotationCenter.push_back(m_rotationCenter[i]);
    object["rotationCenter"] = rotationCenter;
    return object;
}

void Data3d::setBaseObject(const QJsonObject& object)
{
    BaseData::setBaseObject(object);
    auto transform = object.value("transform").toArray();
    vtkNew<vtkMatrix4x4> transformMatrix;
    // convert array to 4x4 matrix
    for (int i = 0; i < transform.size(); i++)
        transformMatrix->SetElement((int)(i / 4), (i % 4),
            transform[i].toDouble());
    m_transform->SetMatrix(transformMatrix);
    m_transform->Update();
    getCoordinate()->setTransform(m_transform->GetMatrix());
    auto rotationCenter = object.value("rotationCenter").toArray();
    for (int i = 0; i < m_rotationCenter.size(); ++i)
        m_rotationCenter[i] = rotationCenter[i].toDouble();
}

} // namespace utility
} // namespace interface
}; // namespace ultrast
