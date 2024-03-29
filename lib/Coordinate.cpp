#include <Coordinate.h>

namespace ultrast {
namespace infrastructure {
namespace utility {

Coordinate::Coordinate(std::string id, const unit::TransformMatrix matrix) :
    m_id(id),
    QObject()
{
    m_transformationMatrix = unit::TransformMatrix::New();
    m_transformationMatrix->DeepCopy(matrix);
}

Coordinate::~Coordinate()
{
}

void Coordinate::setTransform(unit::TransformMatrix matrix)
{
    m_transformationMatrix->DeepCopy(matrix);
    emit transformUpdated(&m_transformationMatrix);
}

}
}
};