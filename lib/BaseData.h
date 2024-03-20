#pragma once

#include <QColor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>

namespace ultrast {
namespace infrastructure {
namespace utility {

enum class DataType : int
{
    INVALID = 0,
    IMAGE2D,
    VOLUME_IMAGE,
    MEDICAL_IMAGE,
    POINT_SET,
    MODEL,
    SURGERY_MODEL,
    SURFACE_MODEL,
    VIDEO,
    MARKER_POINT,
};

class BaseData : public QObject
{
    Q_OBJECT
public:
    BaseData(int id = -1, QString name = QString(), bool hasFilePath = false,
        bool is3d = false);
    /**
    * @brief This function is to be called before delete in m_dataManager
    *
    * The destructor is not appropriate as the destructor is called on software closure, this function is for deletion of data from the data manager
    */
    virtual void onDelete(const bool& modify = true);
    virtual ~BaseData();
    void toggleVisiblity();
    //get methods
    const QString getTypeAsQString() const;
    static const DataType getTypefromQString(QString type);
    const QString getName() const { return m_dataName; }
    const bool isTemporary() const { return m_temporary; }
    const bool isVisible() const { return m_visible; };
    const double getOpacity() const { return m_opacity; };
    const QColor getColor() const { return m_color; };
    const int getSurgeryId() const { return m_surgeryId; }
    const DataType getType() const { return m_type; }
    const bool is3d() const { return m_is3d; }
    const bool hasFilePath() const { return m_hasFilePath; }
    //set methods
    virtual void setTemporary(bool
            temporary); //can consider calling this in temporary data manager when adding data
    virtual void setVisible(bool visible);
    virtual void setOpacity(double opacity);
    virtual void setColor(QColor color);
    virtual void setColor(int r, int g, int b);
    void setSurgeryId(int id);

    //functions used for database handling
    //for writing data to database
    virtual const QJsonDocument toQJsonDocument() const = 0;
    const QString toQJsonDocumentAsQString() const;
    virtual void fromQJsonDocument(QJsonDocument& document) = 0;
    /**
    * @brief debug print purposes
    */
    // const QString toQString() const override;
    /**
    * @brief converts this BaseData instance into a vector of QStrings
    *
    * @return vector of strings in format for database entry
    */
    // const std::vector<QString> toQStringVector() const override;
    // void fromQVariantVector(std::vector<QVariant> parameters) override;

    //overriding database methods to block saving if flag is false

    //functions for data relation
    /**
    * @brief get the ids of BaseData that are children of this instance
    *
    * @return vector of BaseData ids that are children of this instance
    */
    // const std::vector<int> getChildrenIds() const;
    // const std::vector<BaseData*> getChildren() const { return m_children; };
    // const std::vector<BaseData*> get3dChildren() const;
    // const std::vector<BaseData*> get2dChildren() const;
    /**
    * @brief adds a BaseData subclass as a child and updates the child world transform matrix
    *
    * Will override the parent if the child already has a parent
    *
    * @param child The BaseData subclass which can't be itself or the root parent
    * @param modify Will modify the transform to maintain the world position, set to false to retain m_transform
    * @return bool of the result of the operation
    */
    // virtual const bool addChild(BaseData* child, const bool& modify = true);
    /**
    * @brief removes a BaseData subclass from m_children and updates the child world transform matrix
    *
    * Will do nothing if the child does not exist
    *
    * @param child The BaseData subclass which is a child of the current data
    * @param modify Will modify the transform to maintain the world position, set to false to retain m_transform
    * @return bool of the result of the operation
    */
    // virtual const bool removeChild(BaseData* child, const bool& modify = true);

signals:
    void removed(BaseData* self, const bool& modify);
    void propertyUpdated();

protected:
    /**
    * @brief get a QJsonObject with the values of the BaseData variables
    *
    * Gets the variables for m_dataName, m_isVisible, m_transform, and all qProperties in the QJsonObject
    *
    * @return QJsonObject
    */
    virtual QJsonObject getBaseObject() const;
    /**
    * @brief set the BaseData variables with a QJsonObject
    *
    * Sets the variables for m_dataName, m_isVisible, m_transform, and all qProperties from the QJsonObject
    *
    * @param const QJsonObject&
    */
    virtual void setBaseObject(const QJsonObject& object);

    const int getParentId(int id) const;

    int m_surgeryId;
    DataType m_type = DataType::INVALID;
    QString m_dataName;
    bool m_temporary;
    bool m_visible;
    double
        m_opacity; //opacity is separated from color for more control, incase we want objects with 2 colors, etc.
    QColor m_color;
    bool m_is3d;
    bool m_hasFilePath;
    std::vector<BaseData*> m_children;
    BaseData* m_parent;
};

class FileData
{
public:
    FileData(const QString& filePath = QString()) :
        m_filePath(filePath)
    {
    }
    virtual void setFilePath(const QString& filePath)
    {
        m_filePath = filePath;
    };
    const QString getFilePath() const { return m_filePath; };

protected:
    QString m_filePath;
};

} // namespace utility
} // namespace interface
}; // namespace ultrast