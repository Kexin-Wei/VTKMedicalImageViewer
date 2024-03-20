#include <VtkCustomizedButton.h>
#include <qDebug>
// #include <infrastructure\utility\Logger.h>

namespace ultrast {
namespace visualization {

vtkStandardNewMacro(VtkCustomizedButton);

VtkCustomizedButton::VtkCustomizedButton() :
    m_buttonStateOne(vtkSmartPointer<vtkImageData>::New()),
    m_customizedBtnRepresent(
        vtkSmartPointer<vtkTexturedButtonRepresentation2D>::New()),
    m_positionXY(vtkSmartPointer<vtkCoordinate>::New())
{
    m_customizedBtnRepresent->SetNumberOfStates(1);
    m_customizedBtnRepresent->Highlight(
        vtkButtonRepresentation::HighlightHovering);
    this->SetRepresentation(m_customizedBtnRepresent);
}

void VtkCustomizedButton::setupButton(
    const vtkSmartPointer<vtkRenderer>& renderer,
    const vtkSmartPointer<vtkRenderWindowInteractor>& it,
    const QSize& buttonSize, ButtonType mainType,
    ButtonAxisSubType buttonSubType)
{
    this->m_renderer = renderer;
    this->m_it = it;

    this->m_buttonSize = buttonSize;
    this->m_type = mainType;
    this->m_subType = buttonSubType;

    this->setIconResource();
    this->setButtonIcon(m_iconResource);

    m_customizedBtnRepresent->SetButtonTexture(0, m_buttonStateOne);

    this->SetInteractor(m_it);
    this->On();
}

void VtkCustomizedButton::setButtonIcon(QString filename)
{
    QImage* image = new QImage(filename);
    m_iconSize.setWidth(image->width());
    m_iconSize.setHeight(image->height());

    vtkNew<vtkQImageToImageSource> convertToImageData;
    convertToImageData->SetQImage(image);
    convertToImageData->Update();

    m_buttonStateOne->DeepCopy(convertToImageData->GetOutput());
    delete image;
}

void VtkCustomizedButton::repositionOnWindowResize()
{
    int* size = m_renderer->GetRenderWindow()->GetSize();
    double windowWidth = *size;
    double windowHeight = *(size + 1);
    m_positionXY->SetCoordinateSystemToDisplay();

    // set the world coordinate, here just set two arguments, that is x,y.
    // z is set zero by default, so the world coordinate is (btnPosOnViewerWindowX, btnPosOnViewerWindowY, 0)
    // original point in 2D coordinate system of VTK window locates at the left bottom position
    double computeWinPosX, computeWinPosY;
    setButtonPosBySubType(windowWidth, windowHeight, computeWinPosX,
        computeWinPosY);
    m_positionXY->SetValue(computeWinPosX, computeWinPosY);

    // Computes the size of a bounding box. The button occupies the area of this bounding box.
    int* displayCoordinate = m_positionXY->GetComputedDisplayValue(m_renderer);
    m_bounds[0] = displayCoordinate[0] - m_buttonSize.width();
    m_bounds[1] = m_bounds[0] + m_buttonSize.width();
    m_bounds[2] = displayCoordinate[1] - m_buttonSize.height();
    m_bounds[3] = m_bounds[2] + m_buttonSize.height();
    m_bounds[4] = 0.0;
    m_bounds[5] = 0.0;

    m_customizedBtnRepresent->SetPlaceFactor(1);
    m_customizedBtnRepresent->PlaceWidget(m_bounds);
}

void VtkCustomizedButton::setButtonPosBySubType(const double& winWidth,
    const double& winHeight, double& computePosX, double& computePosY)
{
    // this is the number of controlling buttons layout on rendering windows, each number is represented the action for each button.
    // number is also as the ButtonAxisSubType for doing specific logic of controlling the surgery models' movements and resize
    // [4][0][5]    [6]
    // [2][1][3] [8][7][9]
    double spacingBetweenButtons = 4.0f;
    double spacingBetweenGroups = 10.0f;
    double horizontalOffset = 50;
    switch (m_subType)
    {
    case ButtonAxisSubType::UP: // up
    {
        // actually this is the rendering icon width
        computePosX = (winWidth - m_iconSize.width()) / 2.0f + horizontalOffset;
        // From bottom to top, leave spacingBetweenButtons pixesl with the down button between baseline,
        // and leave a blank button then ,leave spacingBetweenButtons + spacingBetweenButtons pixels
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::DOWN:
    {
        computePosX = (winWidth - m_iconSize.width()) / 2.0f + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::LEFT:
    {
        computePosX = (winWidth - winWidth / 2.0f) - m_iconSize.width() / 2.0f
            - spacingBetweenButtons - m_iconSize.width() + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::RIGHT:
    {
        computePosX = (winWidth - winWidth / 2.0f) - m_iconSize.width() / 2.0f
            + spacingBetweenButtons + m_iconSize.width() + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::CLOCKWISE_ROTATE:
    {
        computePosX = (winWidth - winWidth / 2.0f) - m_iconSize.width() / 2.0f
            - spacingBetweenButtons - m_iconSize.width() + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::ANTICLOCKWISE_ROTATE:
    {
        computePosX = (winWidth - winWidth / 2.0f) - m_iconSize.width() / 2.0f
            + spacingBetweenButtons + m_iconSize.width() + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::SIZE_VERTICAL_INCREASE: // up
    {
        computePosX = (winWidth - winWidth / 2.0f) - m_iconSize.width() / 2.0f
            + 2 * spacingBetweenButtons + spacingBetweenGroups
            + 3 * m_iconSize.width() + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::SIZE_VERTICAL_DECREASE:
    {
        computePosX = (winWidth - winWidth / 2.0f) - m_iconSize.width() / 2.0f
            + 2 * spacingBetweenButtons + spacingBetweenGroups
            + 3 * m_iconSize.width() + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::SIZE_HORIZONTAL_DECREASE:
    {
        computePosX = (winWidth - winWidth / 2.0f) - m_iconSize.width() / 2.0f
            + spacingBetweenButtons + spacingBetweenGroups
            + 2 * m_iconSize.width() + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    case ButtonAxisSubType::SIZE_HORIZONTAL_INCREASE:
    {
        computePosX = (winWidth - winWidth / 2.0f) - m_iconSize.width() / 2.0f
            + 3 * spacingBetweenButtons + spacingBetweenGroups
            + 4 * m_iconSize.width() + horizontalOffset;
        computePosY = spacingBetweenButtons + m_iconSize.height()
            + spacingBetweenButtons;
    }
    break;
    default:
        qDebug()
            << "VtkCustomizedButton -- Subtype of surgery model controlling button surpass the predefined number!";
        break;
    }
}

}
}
