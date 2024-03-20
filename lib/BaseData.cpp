#include <BaseData.h>
// #include <Logger.h>
#include <QDebug>
#include <unit.h>

namespace ultrast {
namespace infrastructure {
namespace utility {

BaseData::BaseData(int id, QString name, bool hasFilePath, bool is3d) :
    m_surgeryId(-1),
    m_dataName(name),
    m_temporary(false),
    m_visible(true),
    m_opacity(1.0),
    m_color(0, 0, 0),
    m_hasFilePath(hasFilePath),
    m_is3d(is3d),
    m_parent(nullptr)
{
}

void BaseData::onDelete(const bool& modify)
{
    if (modify)
        //signal to remove from m_children of parent
        //remove self from parent by signal
        emit removed(this, modify);
}

BaseData::~BaseData()
{
}

void BaseData::toggleVisiblity()
{
    setVisible(!m_visible);
}

const QString BaseData::getTypeAsQString() const
{
    switch (m_type)
    {
    case DataType::INVALID:
        return "Invalid";
    case DataType::IMAGE2D:
        return "Image 2d";
    case DataType::VOLUME_IMAGE:
        return "Volume image";
    case DataType::MEDICAL_IMAGE:
        return "Medical image";
    case DataType::POINT_SET:
        return "Point set";
    case DataType::MODEL:
        return "Model";
    case DataType::SURGERY_MODEL:
        return "Surgery model";
    case DataType::VIDEO:
        return "Video";
    case DataType::MARKER_POINT:
        return "Marker Point";
    case DataType::SURFACE_MODEL:
        return "Surface Model";
    }
}

const DataType BaseData::getTypefromQString(QString type)
{
    if (type == "Invalid")
    {
        return DataType::INVALID;
    }
    else if (type == "Image 2d")
    {
        return DataType::IMAGE2D;
    }
    else if (type == "Volume image")
    {
        return DataType::VOLUME_IMAGE;
    }
    else if (type == "Medical image")
    {
        return DataType::MEDICAL_IMAGE;
    }
    else if (type == "Point set")
    {
        return DataType::POINT_SET;
    }
    else if (type == "Model")
    {
        return DataType::MODEL;
    }
    else if (type == "Surgery model")
    {
        return DataType::SURGERY_MODEL;
    }
    else if (type == "Video")
    {
        return DataType::VIDEO;
    }
    else if (type == "Marker Point")
    {
        return DataType::MARKER_POINT;
    }
    else if (type == "Surface Model")
    {
        return DataType::SURFACE_MODEL;
    }
}

void BaseData::setVisible(bool visible)
{
    if (m_visible == visible)
        return;
    m_visible = visible;
    qDebug() << QString("BaseData - visibility set to: %1").arg(visible);
    emit propertyUpdated();
}

void BaseData::setOpacity(double opacity)
{
    if (m_opacity == opacity)
        return;
    m_opacity = opacity;
    qDebug() << QString("BaseData - opacity set to: %1")
                    .arg(QString::number(opacity));
    emit propertyUpdated();
}

void BaseData::setColor(QColor color)
{
    if (m_color == color)
        return;
    m_color = color;
    qDebug() << QString("BaseData - color set to: %1").arg(color.name());
    emit propertyUpdated();
}

void BaseData::setColor(int r, int g, int b)
{
    setColor(QColor(r, g, b));
}

void BaseData::setTemporary(bool temporary)
{
    m_temporary = temporary;
}

void BaseData::setSurgeryId(int id)
{
    m_surgeryId = id;
}

const QString BaseData::toQJsonDocumentAsQString() const
{
    return QString(toQJsonDocument().toJson(QJsonDocument::Compact));
}

// const QString BaseData::toQString() const
// {
//     QString result
//         = QString("\nId: %1\n"
//                   "Surgery Id: %2\n"
//                   "Type: %3\n"
//                   "Json Data: %4")
//               .arg(QString::number(m_id), QString::number(m_surgeryId),
//                   getTypeAsQString(), toQJsonDocumentAsQString());
//     return result;
// }

// const std::vector<QString> BaseData::toQStringVector() const
// {
//     //non numeric data needs text wrap of 'content'
//     std::vector<QString> result;
//     result.push_back(QString::number(m_id));
//     result.push_back(QString::number(m_surgeryId));
//     result.push_back(textWrap(getTypeAsQString()));
//     result.push_back(textWrap(toQJsonDocumentAsQString()));
//     return result;
// }

// void BaseData::fromQVariantVector(std::vector<QVariant> parameters)
// {
//     if (parameters.size() == 3)
//     {
//         m_surgeryId = parameters[0].toInt();
//         m_type = getTypefromQString(parameters[1].toString());
//         QJsonDocument document
//             = QJsonDocument::fromJson(parameters[2].toString().toUtf8());
//         fromQJsonDocument(document);
//     }
//     else
//         Logger::error(tr("BaseData - Invalid QVariant vector size"));
// }

// void BaseData::save()
// {
//     if (m_temporary)
//         return;
//     DatabaseObject::save();
// }

// void BaseData::update()
// {
//     if (m_temporary)
//         return;
//     DatabaseObject::update();
// }

// void BaseData::erase()
// {
//     if (m_temporary)
//         return;
//     DatabaseObject::erase();
// }

// const std::vector<int> BaseData::getChildrenIds() const
// {
//     return DataRelation::getRelationOfData(getId(), DataRelation::CHILD);
// }

// const std::vector<BaseData*>
// ultrast::infrastructure::utility::BaseData::get3dChildren() const
// {
//     std::vector<BaseData*> result;
//     for (auto data : m_children)
//         if (data->is3d())
//             result.push_back(data);
//     return result;
// }

// const std::vector<BaseData*>
// ultrast::infrastructure::utility::BaseData::get2dChildren() const
// {
//     std::vector<BaseData*> result;
//     for (auto data : m_children)
//         if (!data->is3d())
//             result.push_back(data);
//     return result;
// }

// const int BaseData::getParentId(int id) const
// {
//     return DataRelation::getParentIdOfData(id);
// }

// const bool BaseData::addChild(BaseData* child, const bool& modify)
// {
//     Logger::debug(
//         QString("Base Data --- Add child called: parent id %1 and child id %2")
//             .arg(getId())
//             .arg(child->getId()));
//     if (child == this) //child is not self
//         return false;
//     if (std::find(m_children.begin(), m_children.end(), child)
//         != m_children.end()) //child does not already exist
//         return false;
//     //check child id is not root parent id

//     // Do not add if child is ancestor of this
//     auto currId = getParentId(m_id);
//     while (currId != -1)
//     {
//         if (currId == child->getId())
//             return false;
//         currId = getParentId(currId);
//     }
//     if (DataRelation::setRelation(getId(), DataRelation::CHILD, child->getId()))
//     {
//         if (m_type == DataType::SURGERY_MODEL && !(m_children.empty()))
//         {
//             Logger::error(
//                 "BaseData -- Surgery Model Already Has a Pointset, Removing All Now");
//             for (auto child_ : m_children)
//                 removeChild(
//                     child_); // to ensure a surgery_model only has one surgery plan
//         }

//         if (child->m_parent != this && child->m_parent)
//             child->m_parent->removeChild(
//                 child); // remove the current parent if there is
//         m_children.push_back(child);
//         child->m_parent = this;
//         connect(child, &BaseData::removed, this, &BaseData::removeChild,
//             Qt::DirectConnection);
//         Logger::trace(
//             QString("BaseData -- child added (id:%1), called from (id:%2)")
//                 .arg(QString::number(child->getId()))
//                 .arg(getId()));
//         return true;
//     }
//     Logger::error(
//         QString(tr("BaseData -- Unable To Add Root Parent as a Child")));
//     return false;
// }

// const bool BaseData::removeChild(BaseData* child, const bool& modify)
// {
//     if (child)
//     {
//         auto iter = std::find(m_children.begin(), m_children.end(), child);
//         if (iter != m_children.end())
//         { //exists in m_children
//             DataRelation::removeRelation(getId(), DataRelation::CHILD,
//                 child->getId());
//             child->m_parent = nullptr;
//             m_children.erase(iter);
//             if (child)
//                 disconnect(child, &BaseData::removed, this,
//                     &BaseData::removeChild);
//             Logger::trace(QString(
//                 "BaseData -- child removed (id:%1), called from (id:%2)")
//                               .arg(QString::number(child->getId()))
//                               .arg(getId()));
//             return true;
//         }
//     }
//     return false;
// }

QJsonObject BaseData::getBaseObject() const
{
    QJsonObject object;
    object["fileName"] = m_dataName;
    object["visible"] = m_visible;
    object["opacity"] = m_opacity;
    object["color"] = m_color.name(QColor::HexArgb);
    object["is3d"] = m_is3d;
    object["hasFilePath"] = m_hasFilePath;
    //store all qProperties
    QJsonObject properties;
    auto propertyNames = this->dynamicPropertyNames();
    for (auto propertyName : propertyNames)
    {
        QString propertyString = QString(propertyName);
        properties[propertyString] = QJsonValue::fromVariant(
            this->property(propertyString.toStdString().c_str()));
    }
    object["properties"] = properties;
    return object;
}

void BaseData::setBaseObject(const QJsonObject& object)
{
    m_dataName = object.value("fileName").toString();
    m_visible = object.value("visible").toBool();
    m_opacity = object.value("opacity").toDouble();
    m_color = QColor(object.value("color").toString());
    m_is3d = object.value("is3d").toBool();
    m_hasFilePath = object.value("hasFilePath").toBool();
    //load all qProperties
    QJsonObject properties = object["properties"].toObject();
    for (auto property : properties.keys())
        this->setProperty(property.toStdString().c_str(),
            properties[property].toVariant());
}

} // namespace utility
} // namespace interface
}; // namespace ultrast
