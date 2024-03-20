#pragma once
#include <QString>
#include <string>
#include <map>
#include <vtkTransform.h>
#include "unit.h"

namespace ultrast {
namespace infrastructure {
namespace utility {

class Coordinate;

class CoordinateRepository {
protected:
    CoordinateRepository();
    virtual ~CoordinateRepository();
public:
    static CoordinateRepository* getInstance();
    Coordinate* createCoordinate(std::string id, std::string parent = "", const unit::TransformMatrix matrix = unit::TransformMatrix::New());
    Coordinate* getCoordinate(std::string id) const;
    const unit::TransformMatrix getTransform(std::string from, std::string to);
	const vtkSmartPointer<vtkTransform> getVtkTransform(std::string from, std::string to);
    const bool setCoordinateParent(const std::string& id, const std::string& parent);
    const bool deleteCoordinate(const std::string id);
    void clearCoordinates();
    /**
    * @brief Gets the path of string keys from the coordinate map based on common parents
    *
    * @param from a key in the coordinate map
    * @param to another key from the coordinate map
    * @param commonParentToFrom ordered vector of string keys from the common parent to the key of 'from' parameter
    * @param commonParentToTo ordered vector of string keys from the common parent to the key of 'to' parameter
    */
    void getPath(std::string from, std::string to, std::vector<std::string>& commonParentToFrom, std::vector<std::string>& commonParentToTo);
    void transformPoint(QString from, QString to, double in[3], double(&out)[3]);
    void transformPoint(std::string from, std::string to, unit::Point& point);

    /**
    * @brief Gets the path of string keys from the coordinate map based on common parents
    *
    * @param id CoordinateId of the data we getting parent transform of 
    */
    unit::TransformMatrix getParentWorldTransform(std::string id);
    std::string getParentCoodinateId(std::string id);
    
    /**
    * @brief Resolve the transformation matrix to euler angle.
    * Rotation order refers to  http://www.songho.ca/opengl/gl_anglestoaxes.html
    * Function body refers to https://github.com/spmallick/learnopencv/blob/master/RotationMatrixToEulerAngles/rotm2euler.cpp
    * alpha: rotation about X-axis, pitch
    * beta: rotation about Y-axis, yaw(heading)
    * gamma: rotation about Z-axis, roll
    * In general, we use Rgamma * Rbeta * Ralpha, Z * Y * X
    * |Cz -Sz 0| | Cy  0 Sy| |1  0   0|   | cos(gamma)cos(beta) - sin(gamma)cos(alpha) + cos(gamma)sin(beta)sin(alpha)   sin(gamma)sin(alpha) + cos(gamma)sin(beta)cos(alpha)|
    * |Sz  Cz 0|*| 0  1  0 |*|0 Cx -Sx|	= | sin(gamma)cos(beta)   cos(gamma)cos(alpha)+sin(gamma)sin(beta)sin(alpha)   -cos(gamma)sin(alpha)+sin(gamma)sin(beta)cos(alpha)|
    * | 0  0  1| |-Sy  0 Cy| |0 Sx  Cx|   |      -sin(beta)                   cos(beta)sin(alpha)                               cos(beta)cos(alpha)                       |
    * @param matrix
    * @return bool and poseEuler
    */
    static bool transformToPoseEuler(unit::TransformMatrix matrix, unit::PoseEuler& poseEuler);
    /**
    * @brief Construct the transformation matrix from Euler angles.
    * @param poseEuler
    * @return matrix
    */
    static void PoseEulerToTransform(unit::PoseEuler poseEuler, unit::TransformMatrix& matrix);

    /** Split transform matrix from config file
    * @param transform Label of the transformation from and to for log purposes
    * @param transformMatrix Transform from the config file that is expected to be comma separated
    * @param out Pass by reference of the output array for the transform matrix
    * @return The success status of the function
    **/
    static bool splitTransformMatrix(QString transform, QString transformMatrix, double(&out)[16]);
private:
    const bool coordinateIdExists(const std::string& id) const;

    static CoordinateRepository* m_instance;
    std::map<std::string, Coordinate*> m_mapCoordinates;
    std::map<std::string, std::string> m_mapChildToParent;
};

}
}
};
