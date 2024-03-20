/**
 * @file BaseOverlayButton.cpp
 * @author your name (you@domain.com)
 * @brief Base Class for vtk or qt buttons.
 * This class contains the common functions and variables for both the classes.
 * 
 * @version 0.1
 * @date 2021-11-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <BaseOverlayButton.h>

namespace ultrast {
namespace visualization {

BaseOverlayButton::BaseOverlayButton(ButtonPosition position, ButtonType type,
    double size) :
    m_position(position),
    m_type(type),
    m_size(size)
{
    this->setIconResource();
}

BaseOverlayButton::BaseOverlayButton()
{
}

BaseOverlayButton::~BaseOverlayButton()
{
}

void BaseOverlayButton::setIconResource()
{
    switch (m_type)
    {
    case ButtonType::Resize:
        m_iconResource = ":/resources/icons/resize_48";
        break;
    case ButtonType::Menu:
        m_iconResource = ":/resources/icons/menu_32";
        break;
    case ButtonType::Export:
        m_iconResource = ":/resources/icons/capture_48";
        break;
    case ButtonType::ImageInfo:
        m_iconResource = ":/resources/icons/info_48";
        break;
    case ButtonType::TransducerCrosshair:
        m_iconResource = ":/resources/icons/focus_48";
        break;
    case ButtonType::MeasureLength:
        m_iconResource = ":/resources/icons/measure_length_48";
        break;
    case ButtonType::MeasureAngle:
        m_iconResource = ":/resources/icons/measure_angle_48";
        break;
    case ButtonType::DrawEllipse:
        m_iconResource = ":/resources/icons/circle_white_48";
        break;
    case ButtonType::DrawRectangle:
        m_iconResource = ":/resources/icons/rectangle_48";
        break;
    case ButtonType::DrawTrace:
        m_iconResource = ":/resources/icons/dotted_48";
        break;
    case ButtonType::ShowMeasurement:
        m_iconResource = ":/resources/icons/eye_show_48";
        m_iconResourceNotChecked = ":/resources/icons/eye_hide_48";
        break;
    case ButtonType::Trash:
        m_iconResource = ":/resources/icons/trash_icon_32";
        break;
    case ButtonType::Close:
        m_iconResource = ":/resources/icons/close_48";
        break;
    case ButtonType::IgtlSend:
        m_iconResource = ":/resources/icons/info_48";
        break;
    case ButtonType::Zoom:
        m_iconResource = ":/resources/icons/zoom_in";
        m_iconResourceNotChecked = ":/resources/icons/zoom_out";
        break;
    case ButtonType::ResizeRawImage:
        m_iconResource = ":/resources/icons/raw_image";
        m_iconResourceNotChecked = ":/resources/icons/resized_image";
        break;
    case ButtonType::GridLines:
        m_iconResource = ":/resources/icons/grid_lines";
        break;
    }
}

BaseCustomizedButton::BaseCustomizedButton(QSize buttonSize, ButtonType type) :
    m_type(type),
    m_buttonSize(buttonSize)
{
}

void BaseCustomizedButton::setIconResource()
{
    switch (m_subType)
    {
    case UP:
        m_iconResource = ":/resources/icons/up_move";
        m_iconResourceNotChecked = ":/resources/icons/up_move";
        break;
    case DOWN:
        m_iconResource = ":/resources/icons/down_move";
        m_iconResourceNotChecked = ":/resources/icons/down_move";
        break;
    case LEFT:
        m_iconResource = ":/resources/icons/left_move";
        m_iconResourceNotChecked = ":/resources/icons/left_move";
        break;
    case RIGHT:
        m_iconResource = ":/resources/icons/right_move";
        m_iconResourceNotChecked = ":/resources/icons/right_move";
        break;
    case ANTICLOCKWISE_ROTATE:
        m_iconResource = ":/resources/icons/anti_clock";
        m_iconResourceNotChecked = ":/resources/icons/anti_clock";
        break;
    case CLOCKWISE_ROTATE:
        m_iconResource = ":/resources/icons/clock";
        m_iconResourceNotChecked = ":/resources/icons/clock";
        break;
    case SIZE_VERTICAL_INCREASE:
        m_iconResource = ":/resources/icons/vertical_increase";
        m_iconResourceNotChecked = ":/resources/icons/vertical_increase";
        break;
    case SIZE_VERTICAL_DECREASE:
        m_iconResource = ":/resources/icons/vertical_decrease";
        m_iconResourceNotChecked = ":/resources/icons/vertical_decrease";
        break;
    case SIZE_HORIZONTAL_DECREASE:
        m_iconResource = ":/resources/icons/horizontal_decrease";
        m_iconResourceNotChecked = ":/resources/icons/horizontal_decrease";
        break;
    case SIZE_HORIZONTAL_INCREASE:
        m_iconResource = ":/resources/icons/horizontal_increase";
        m_iconResourceNotChecked = ":/resources/icons/horizontal_increase";
        break;
    default:
        break;
    }
}

}
}