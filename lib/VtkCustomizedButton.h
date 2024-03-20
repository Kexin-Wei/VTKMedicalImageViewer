/**
 * @file VtkCustomizedButton.h
 * @author TaoShang (shang.tao@ultrastmedtech.com)
 * @brief Derived from vtkButtonWidget. 
 * The customized button is meant to be rendered on an opengl renderer.
 * After instantiating pass the renderer to the instance.
 * Set the ButtonType and Position
 * 
 * The interactor must be set before the button is turned on.
 * VtkCustomizedButton->SetInteractor(m_it);
 * VtkCustomizedButton->On();
 * 
 * @version 0.1
 * @date 2022-07-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include "BaseOverlayButton.h"
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <array>
#include <vtkButtonWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkCoordinate.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkObjectFactory.h>
#include <vtkPNGReader.h>
#include <vtkQImageToImageSource.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTexture.h>
#include <vtkTexturedButtonRepresentation2D.h>

namespace ultrast {
namespace visualization {

class VtkCustomizedButton : public vtkButtonWidget, public BaseCustomizedButton
{
public: // member functions
    static VtkCustomizedButton* New();
    vtkTypeMacro(VtkCustomizedButton, vtkButtonWidget);
    /**
        * @brief This sets up the button for display
        * @param renderer
        * @param it
        * @param btnSize
        * @param btnSubType,0 is up, 1 is down, 2 is left, 3 is right, 4 is antiClock rotate, 5 is clock rotate
        */
    void setupButton(const vtkSmartPointer<vtkRenderer>& renderer,
        const vtkSmartPointer<vtkRenderWindowInteractor>& it,
        const QSize& btnSize, ButtonType mainType,
        ButtonAxisSubType btnSubType);
    /**
        * @brief Repositions the button on window resize.
        */
    void repositionOnWindowResize();

protected: // variables
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkRenderWindowInteractor> m_it;
    vtkSmartPointer<vtkTexturedButtonRepresentation2D> m_customizedBtnRepresent;

    vtkSmartPointer<vtkImageData> m_buttonStateOne;
    vtkSmartPointer<vtkCoordinate> m_positionXY;

protected: // functions
    /**
        * @brief Construct a new Vtk Customized Button:: Vtk Customized Button object.
        * This uses the parameterless, overloaded constructor for baseoverlaybutton.
        */
    VtkCustomizedButton();
    ~VtkCustomizedButton() override = default;

private:
    /**
        * @brief set button position by sub type, sub type is the number of control widget repsenting the
        *        movement direction: Up is 0, down is 1, left is 2, right is 3, clockwisze rotation is 4, anti-clockwize rotation is 5
        * @param winWidth rendering window width
        * @param winHeight rendering window height
        * @param computePosX comput the X coordinate with left bottom of every button and return this value
        * @param computePosY comput the Y coordinate with left bottom of every button and return this value
        */
    void setButtonPosBySubType(const double& winWidth, const double& winHeight,
        double& computePosX, double& computePosY);
    /**
        * @brief converts QImage to VtkImageData
        *        and set the icon for ButtonRepresentation
        * @param filename
        */
    void setButtonIcon(QString filename);
    VtkCustomizedButton(const VtkCustomizedButton&) = delete;
    void operator=(const VtkCustomizedButton&) = delete;
    double m_bounds[6];
    QSize m_iconSize;
};

}
};
