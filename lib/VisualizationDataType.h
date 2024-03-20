/**
 * @file   VisualizationDataType.h
 *
 * @brief  class that contains the enums and structs used for visualization methods
 *
 * @author Nicholas
 * Contact: nicholas.ng@ultrastmedtech.com
 * @date   21/09/2021
 */
#pragma once

#include <QString>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkBox.h>

namespace ultrast {
namespace infrastructure {
namespace utility {

#define TRANSDUCER_COLOR(alpha) QColor(255,255,0, alpha)
#define DARKER_GRAY QColor(40,40,40,255)
#define TITLE_COLOR QColor(11,180,214,255)

#define SYSTEM_MARKER_COLOR_NAME std::string("yellow")
#define TARGET_MARKER_COLOR_NAME std::string("orange")
#define US_MARKER_COLOR_NAME std::string("mediumorchid")
#define DICOM_MARKER_COLOR_NAME std::string("dodgerblue")
#define MARKER_BIOPSY_COLOR_NAME std::string("lime")
#define MARKER_ROTATION_CENTER_COLOR_NAME std::string("pink")

enum VisualAxis : unsigned int {
    X = 0,
    Y = 1,
    Z = 2
};

enum MeasurementType {
    NONE,
    DISTANCE,
    ANGLE,
    VOLUME
};

enum ViewerType {
    INVALID,
    SLICE,
    STEREO,
    IMAGE,
    MEDIA,
    VIDEO
};

struct ViewerInfo {
    QString m_viewerId;
    VisualAxis m_third;
    ViewerType m_type;
    ViewerInfo(QString id, ViewerType type, VisualAxis third = VisualAxis::Z) :
        m_viewerId(id),
        m_type(type),
        m_third(third)
    {
    };
};

struct FlexibleSliceDataInfo {
    int planeWidth = 0;
    int planeHeight = 0;
    vtkMatrix4x4* resliceAxes = nullptr;
    vtkMatrix4x4* flipMatrix = nullptr;
};

} // namespace data
} // namespace user_interface
}; // namespace visualization