#define _USE_MATH_DEFINES
#include <math.h>

#include <QDebug>
#include <vtkTransform.h>

#include <Coordinate.h>
#include <CoordinateRepository.h>

namespace ultrast {
namespace infrastructure {
namespace utility {

CoordinateRepository* CoordinateRepository::m_instance = nullptr;

CoordinateRepository* CoordinateRepository::getInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new CoordinateRepository();
    }
    return m_instance;
}

Coordinate* CoordinateRepository::createCoordinate(std::string id,
    std::string parent, const unit::TransformMatrix matrix)
{
    auto it = m_mapCoordinates.find(id);
    if (it != m_mapCoordinates.end())
    {
        qDebug() << "CoordinateRepository -- Coordinate id: "
                 << QString::fromStdString(id) << " already exists.";
        return nullptr;
    }
    it = m_mapCoordinates.find(parent);
    if (it == m_mapCoordinates.end() && parent != "")
    {
        qDebug() << "CoordinateRepository -- Parent coordinate id: "
                 << QString::fromStdString(parent) << " does not exist.";
        return nullptr;
    }

    Coordinate* coordinate = new Coordinate(id, matrix);
    m_mapCoordinates[id] = coordinate;
    m_mapChildToParent[id] = parent;
    return coordinate;
}

Coordinate* CoordinateRepository::getCoordinate(std::string id) const
{
    auto it = m_mapCoordinates.find(id);
    if (it != m_mapCoordinates.end())
        return m_mapCoordinates.at(id);
    qDebug() << "CoordinateRepository -- Coordinate id: "
             << QString::fromStdString(id) << " does not exist.";
    return nullptr;
}

const unit::TransformMatrix CoordinateRepository::getTransform(std::string from,
    std::string to)
{
    std::vector<std::string> commonParentToFrom, commonParentToTo;
    this->getPath(from, to, commonParentToFrom, commonParentToTo);

    if (commonParentToFrom.empty() || commonParentToTo.empty())
        return nullptr;
    auto result = unit::TransformMatrix::New();
    for (size_t i = commonParentToFrom.size() - 1; i > 0; --i)
    {
        unit::TransformMatrix mediumMatrix = unit::TransformMatrix::New();
        unit::TransformMatrix invertMatrix = unit::TransformMatrix::New();
        invertMatrix->DeepCopy(
            m_mapCoordinates.at(commonParentToFrom.at(i))->getTransform());
        invertMatrix->Invert();
        mediumMatrix->DeepCopy(result);
        vtkMatrix4x4::Multiply4x4(mediumMatrix, invertMatrix, result);
    }

    for (size_t i = 1; i < commonParentToTo.size(); ++i)
    {
        unit::TransformMatrix mediumMatrix = unit::TransformMatrix::New();
        mediumMatrix->DeepCopy(result);
        vtkMatrix4x4::Multiply4x4(mediumMatrix,
            m_mapCoordinates.at(commonParentToTo.at(i))->getTransform(),
            result);
    }
    auto resultArray = result->GetData();
    return result;
}

const vtkSmartPointer<vtkTransform> CoordinateRepository::getVtkTransform(
    std::string from, std::string to)
{
    auto matrix = getTransform(to, from);
    if (matrix)
    {
        vtkSmartPointer<vtkTransform> transform = vtkTransform::New();
        transform->SetMatrix(matrix);
        return transform;
    }
    else
        qDebug() << "CoordinateRepository -- Invalid Coordinate";
    return nullptr;
}

const bool CoordinateRepository::setCoordinateParent(const std::string& id,
    const std::string& parent)
{
    //check ids already exist
    if (!coordinateIdExists(id))
        return false;
    if (!coordinateIdExists(parent))
        return false;
    //change parent
    m_mapChildToParent[id] = parent;
    return true;
}

const bool CoordinateRepository::deleteCoordinate(const std::string id)
{
    //check if id exists
    if (coordinateIdExists(id))
    {
        //delete coordinate
        delete m_mapCoordinates[id];
        m_mapCoordinates.erase(id);
        //check if any child has this coordinate as parent
        for (const auto& pair : m_mapChildToParent)
            if (pair.second == id)
                m_mapChildToParent[pair.first]
                    = "world"; //sets parent to world when parent gets deleted
        return true;
    }
    return false;
}

void CoordinateRepository::clearCoordinates()
{
    for (auto& pair : m_mapCoordinates)
    {
        delete pair.second;
        pair.second = nullptr;
    }
    m_mapCoordinates.clear();
    m_mapChildToParent.clear();
    createCoordinate("world");
}

void CoordinateRepository::getPath(std::string from, std::string to,
    std::vector<std::string>& commonParentToFrom,
    std::vector<std::string>& commonParentToTo)
{
    auto it = m_mapCoordinates.find(from);
    if (it == m_mapCoordinates.end())
    {
        return;
    }
    it = m_mapCoordinates.find(to);
    if (it == m_mapCoordinates.end())
    {
        return;
    }

    std::vector<std::string> pathRootToFrom;
    auto node = from;
    while (node != "")
    {
        pathRootToFrom.insert(pathRootToFrom.begin(), node);
        node = m_mapChildToParent.at(node);
    }
    std::vector<std::string> pathRootToTo;
    node = to;
    while (node != "")
    {
        pathRootToTo.insert(pathRootToTo.begin(), node);
        node = m_mapChildToParent.at(node);
    }

    std::string commonParent = "";
    size_t indexFrom = 0;
    size_t indexTo = 0;
    for (size_t i = 0; i < pathRootToFrom.size(); ++i)
        for (size_t j = 0; j < pathRootToTo.size(); ++j)
        {
            if (pathRootToFrom.at(i) == pathRootToTo.at(j))
            {
                commonParent = pathRootToFrom.at(i);
                indexFrom = i;
                indexTo = j;
            }
        }
    if (commonParent == "")
    {
        qDebug() << "CoordinateRepository -- No path found From Coordinate id: "
                 << QString::fromStdString(from)
                 << " To Coordinate id: " << QString::fromStdString(to);
        return;
    }

    commonParentToFrom
        = { pathRootToFrom.begin() + indexFrom, pathRootToFrom.end() };
    commonParentToTo = { pathRootToTo.begin() + indexTo, pathRootToTo.end() };
}

void CoordinateRepository::transformPoint(QString from, QString to,
    double in[3], double (&out)[3])
{
    auto matrix = getTransform(to.toStdString(), from.toStdString());
    if (matrix != nullptr)
    {
        vtkSmartPointer<vtkTransform> transform = vtkTransform::New();
        transform->SetMatrix(matrix);
        transform->TransformPoint(in, out);
        qDebug() << "CoordinateRepository -- Transform point relative from "
                 << from << " (" << in[0] << "," << in[1] << "," << in[2]
                 << ") to " << to << " (" << out[0] << "," << out[1] << ","
                 << out[2] << ")";
    }
    else
        qDebug() << "CoordinateRepository -- Invalid Coordinate";
}

void CoordinateRepository::transformPoint(std::string from, std::string to,
    unit::Point& point)
{
    if (auto transform = getVtkTransform(from, to))
    {
        double coord[3];
        point.toArray(coord);
        //transform point function can't accept our custom Point type so a temp array is necessary
        transform->TransformPoint(coord, coord);
        point = coord;
    }
}

unit::TransformMatrix CoordinateRepository::getParentWorldTransform(
    std::string id)
{
    std::string parentCoordinateId = m_mapChildToParent[id];
    return m_mapCoordinates[parentCoordinateId]->getTransform();
}

std::string CoordinateRepository::getParentCoodinateId(std::string id)
{
    return m_mapChildToParent[id];
}

bool CoordinateRepository::transformToPoseEuler(unit::TransformMatrix matrix,
    unit::PoseEuler& poseEuler)
{
    // check if the matrix is a valid rotation matrix, because matrix is a 4*4 homogeneous matrix
    //we just use the 3*3 rotation matrix without the translation matrix
    //1. first check if the last element is 1
    vtkMatrix4x4* vtkMat = matrix.GetPointer();
    if (vtkMat->Element[3][3] - 1.0f >= 1e-6)
        return false;
    //2. rotation matrix is orthogonal matrix, that means inverse matrix of the matirx equals its transpose matrix
    unit::RotationMatrix rotatioinMat
        = unit::RotationMatrix::New(); //3x3 matrix
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            rotatioinMat->SetElement(i, j, vtkMat->Element[i][j]);
        }
    unit::RotationMatrix rotTranPoseMat
        = unit::RotationMatrix::New(); // transpose matrix of rotation matrix
    rotatioinMat->Transpose(rotatioinMat, rotTranPoseMat);

    unit::RotationMatrix result = unit::RotationMatrix::New();
    vtkMatrix3x3::Multiply3x3(rotatioinMat, rotTranPoseMat,
        result); // result should be a identiy matrix
    if (result->Determinant() - 1.0f
        >= 1e-6) //determinant of result matrix should be 1
        return false;

    // 3. get alpha, beta, gamma angle
    //check if the matrix is singular matrix, if so, z = 0
    double sBeta
        = sqrt(rotatioinMat->GetElement(0, 0) * rotatioinMat->GetElement(0, 0)
            + rotatioinMat->GetElement(1, 0) * rotatioinMat->GetElement(1, 0));

    bool singular = sBeta < 1e-6;
    double radianAlpha, radianBeta, radianGamma;
    if (!singular)
    {
        radianAlpha = atan2(rotatioinMat->GetElement(2, 1),
            rotatioinMat->GetElement(2, 2));
        radianBeta = atan2(-rotatioinMat->GetElement(2, 0), sBeta);
        radianGamma = atan2(rotatioinMat->GetElement(1, 0),
            rotatioinMat->GetElement(0, 0));
    }
    else
    {
        radianAlpha = atan2(-rotatioinMat->GetElement(1, 2),
            rotatioinMat->GetElement(1, 1));
        radianBeta = atan2(-rotatioinMat->GetElement(2, 0), sBeta);
        radianGamma = 0;
    }

    std::array<unit::Degree, 3> degree = { radianAlpha * (180.0 / M_PI),
        radianBeta * (180.0 / M_PI), radianGamma * (180.0 / M_PI) };

    poseEuler.orientation = degree;
    poseEuler.position = unit::Point(vtkMat->GetElement(0, 3),
        vtkMat->GetElement(1, 3), vtkMat->GetElement(2, 3));
    return true;
}

void CoordinateRepository::PoseEulerToTransform(unit::PoseEuler poseEuler,
    unit::TransformMatrix& matrix)
{
    unit::RotationMatrix rotationMatrixAlpha = unit::RotationMatrix::New();
    unit::RotationMatrix rotationMatrixBeta = unit::RotationMatrix::New();
    unit::RotationMatrix rotationMatrixGamma = unit::RotationMatrix::New();

    // Calculate rotation about x axis
    double radianAlpha = (poseEuler.orientation[0] * M_PI) / 180.0f;
    double radianBeta = (poseEuler.orientation[1] * M_PI) / 180.0f;
    double radianGamma = (poseEuler.orientation[2] * M_PI) / 180.0f;

    double alphaMatrix[9] = { 1, 0, 0, 0, cos(radianAlpha), -sin(radianAlpha),
        0, sin(radianAlpha), cos(radianAlpha) };
    rotationMatrixAlpha->DeepCopy(alphaMatrix);

    // Calculate rotation about y axis
    double betaMatrix[9] = { cos(radianBeta), 0, sin(radianBeta), 0, 1, 0,
        -sin(radianBeta), 0, cos(radianBeta) };
    rotationMatrixBeta->DeepCopy(betaMatrix);
    // Calculate rotation about z axis
    double gammaMatrix[9] = { cos(radianGamma), -sin(radianGamma), 0,
        sin(radianGamma), cos(radianGamma), 0, 0, 0, 1 };
    rotationMatrixGamma->DeepCopy(gammaMatrix);
    // Combined rotation matrix
    unit::RotationMatrix tempMatrix = unit::RotationMatrix::New();
    vtkMatrix3x3::Multiply3x3(rotationMatrixBeta, rotationMatrixAlpha,
        tempMatrix); //tempMatrix = rotationMatrixBeta * rotationMatrixAlpha
    vtkMatrix3x3::Multiply3x3(rotationMatrixGamma, tempMatrix,
        tempMatrix); // tempMatrix = rotationMatrixGamma *tempMatrix = rotationMatrixGamma * rotationMatrixBeta * rotationMatrixAlpha

    //construct the matrix
    //translatioin matrix
    for (int i = 0; i < 3; i++)
    {
        matrix->SetElement(i, 3, poseEuler.position[i]);
    }

    double mmmmm[9] = { 0 };
    double* ppp = tempMatrix->GetData();
    double e00 = ppp[0];
    double e01 = ppp[1];
    double e02 = ppp[2];

    double e03 = ppp[3];
    double e04 = ppp[4];
    double e05 = ppp[5];

    //rotation matrix
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            matrix->SetElement(i, j, tempMatrix->GetElement(i, j));
        }
    //left elements
    matrix->SetElement(3, 0, 0);
    matrix->SetElement(3, 1, 0);
    matrix->SetElement(3, 2, 0);
    matrix->SetElement(3, 3, 1.0f);
}

bool CoordinateRepository::splitTransformMatrix(QString transform,
    QString transformMatrix, double (&out)[16])
{
    qDebug() << "CoordinateRepository -- " << transform
             << " -- Transform matrix: " << transformMatrix;
    QStringList list
        = transformMatrix.split(QRegExp(","), QString::SkipEmptyParts);

    if (list.size() != 16)
    {
        qDebug() << "CoordinateRepository -- " << transform
                 << " Transform matrix size is incorrect: " << list.size();
        return false;
    }
    for (int i = 0; i < list.size(); i++)
        out[i] = list.at(i).toDouble();

    qDebug() << "CoordinateRepository -- Split transform matrix of "
             << transform << " :";
    qDebug() << QString("CoordinateRepository -- %1 %2 %3 %4")
                    .arg(QString::number(out[0]), QString::number(out[1]),
                        QString::number(out[2]), QString::number(out[3]));
    qDebug() << QString("CoordinateRepository -- %1 %2 %3 %4")
                    .arg(QString::number(out[4]), QString::number(out[5]),
                        QString::number(out[6]), QString::number(out[7]));
    qDebug() << QString("CoordinateRepository -- %1 %2 %3 %4")
                    .arg(QString::number(out[8]), QString::number(out[9]),
                        QString::number(out[10]), QString::number(out[11]));
    qDebug() << QString("CoordinateRepository -- %1 %2 %3 %4")
                    .arg(QString::number(out[12]), QString::number(out[13]),
                        QString::number(out[14]), QString::number(out[15]));
    return true;
}

const bool CoordinateRepository::coordinateIdExists(const std::string& id) const
{
    if (m_mapCoordinates.find(id) == m_mapCoordinates.end() && id != "")
        return false;
    return true;
}

CoordinateRepository::CoordinateRepository()
{
    createCoordinate("world");
}

CoordinateRepository::~CoordinateRepository()
{
    for (auto& pair : m_mapCoordinates)
    {
        delete pair.second;
        pair.second = nullptr;
    }
}

}
}
}; // namespace ultrast
