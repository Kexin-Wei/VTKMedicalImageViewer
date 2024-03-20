#pragma once
#include <string>
#include <vector>
#include <vtkTransform.h>
#include <QObject>
#include "unit.h"

namespace ultrast {
namespace infrastructure {
namespace utility {
class Coordinate : public QObject {
    Q_OBJECT
    friend class CoordinateRepository;
protected:
    Coordinate(std::string id, unit::TransformMatrix matrix = unit::TransformMatrix::New());
public:
    virtual ~Coordinate();
    const std::string& getId() const { return m_id; }
    void setTransform(unit::TransformMatrix matrix);
    const unit::TransformMatrix getTransform() const { return m_transformationMatrix; }
signals:
    void transformUpdated(const unit::TransformMatrix* transform);
private:
    unit::TransformMatrix m_transformationMatrix;
    std::string m_id;
};

}
}
};