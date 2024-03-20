#pragma once

#include <QDebug>
#include <array>
// #include <infrastructure\utility\Logger.h>
#include <vector>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>

namespace ultrast {
namespace infrastructure {
namespace utility {
namespace unit {
const int MM_PER_CM = 10;

using Millimeters = double;
using Centimeters = double;
using Meters = double;
using Grams = double;
using Kilograms = double;
using Pixel = int;
using Hz = double;
using KHz = double;
using MVpp = double;
using Percentage = double;
using Minutes = int;
using Seconds = double;
using Millisecond = double;
using Microsecond = double;
using Temperature = double;
using Degree = double;
using dBm = double;
using GB = qint64;
using TransformMatrix = vtkSmartPointer<vtkMatrix4x4>;
using RotationMatrix = vtkSmartPointer<vtkMatrix3x3>;
using RotationAngle = std::array<Degree, 3>;

using EncoderCount = std::vector<double>;
using EncoderErrorValue = std::vector<double>;
using LinearStage = std::vector<double>;
using Quaternion = std::array<Millimeters, 4>;
using JointValue = std::vector<double>;

struct Point
{
    Millimeters x, y, z;
    Point(Millimeters x = 0, Millimeters y = 0, Millimeters z = 0) :
        x(x),
        y(y),
        z(z),
        m_invalid(0)
    {
    }
    Point(Millimeters coord[3]) :
        x(coord[0]),
        y(coord[1]),
        z(coord[2]),
        m_invalid(0)
    {
    }

    const int size() const { return 3; }

    void toArray(Millimeters (&coord)[3]) const
    {
        coord[0] = x;
        coord[1] = y;
        coord[2] = z;
    }

    Millimeters& at(int i)
    {
        switch (i)
        {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        }
        qDebug() << "Point -- Attemp to access element index " << i
                 << " is out of bounds.";
        m_invalid = 0;
        return m_invalid;
    }

    const Millimeters& at(int i) const
    {
        switch (i)
        {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        }
        qDebug() << "Point -- Attemp to access element index " << i
                 << " is out of bounds.";
        return 0;
    }

    Millimeters& operator[](int i) { return at(i); }

    const Millimeters operator[](int i) const { return at(i); }

    const bool operator==(const Point& point) const
    {
        auto equal = [](Millimeters a, Millimeters b) -> bool {
            return fabs(a - b) < std::numeric_limits<Millimeters>::epsilon();
        };
        if (equal(x, point.x))
            if (equal(y, point.y))
                return (equal(z, point.z));
        return false;
    }

    const bool operator!=(const Point& point) const
    {
        return !operator==(point);
    }

    Point operator-(const Point& point) const
    {
        Point result;
        result.x = this->x - point.x;
        result.y = this->y - point.y;
        result.z = this->z - point.z;
        return result;
    }

    Point operator+(const Point& point) const
    {
        Point result;
        result.x = this->x + point.x;
        result.y = this->y + point.y;
        result.z = this->z + point.z;
        return result;
    }

    Point operator*(const double& scale) const
    {
        Point result;
        result.x = this->x * scale;
        result.y = this->y * scale;
        result.z = this->z * scale;
        return result;
    }

    Point operator/(const double& scale) const
    {
        Point result;
        result.x = this->x / scale;
        result.y = this->y / scale;
        result.z = this->z / scale;
        return result;
    }

    const QString toQString() const
    {
        return QString("(x: %1, y: %2, z: %3)").arg(x).arg(y).arg(z);
    }

private:
    Millimeters m_invalid;
};

struct Pose
{
    Point position;
    Quaternion orientation;
};

struct PoseEuler
{
    Point position;
    RotationAngle orientation; // alpha, beta, gamma in degree system
};

struct Twist
{
    std::array<Millimeters, 3> linear;
    std::array<Millimeters, 3> angular;
};
}
Q_DECLARE_METATYPE(unit::Point)
}
}
}