#include <BaseViewer.h>
#include <QOverlayButton.h>
#include <VolumeImage.h>
// #include <VtkOverlayButton.h>

using DataType = ultrast::infrastructure::utility::DataType;

namespace ultrast {
namespace visualization {

int BaseViewer::m_idCounter = 0;

BaseViewer::BaseViewer(QWidget* parent, QString name) :
    QFrame(parent),
    m_viewerId("Viewer" + QString::number(++m_idCounter))
{
    this->setObjectName("viewerFrame");
    this->setMinimumHeight(0);
    this->setMinimumWidth(0);
}
BaseViewer::~BaseViewer()
{
}

ViewerInfo BaseViewer::getViewerInfo() const
{
    return ViewerInfo(m_viewerId, m_type);
}

void BaseViewer::addData(Data3d& data)
{
    data.setUpForViewer(getViewerInfo());
    switch (data.getType())
    {
    case DataType::VOLUME_IMAGE:
        addVolumeImage((VolumeImage&)data);
        break;
    }
    render();
}

void BaseViewer::addTextActor(Data3d& data)
{
}

void BaseViewer::removeTextActor(Data3d& data)
{
}

void BaseViewer::removeData(Data3d& data)
{
    switch (data.getType())
    {
    case DataType::VOLUME_IMAGE:
        removeVolumeImage((VolumeImage&)data);
        break;
    }
    render();
}

/**
 * @brief resizeSelf is called whenever the overlay button
 * is pressed for resizing the viewer
 * 
 */
// void BaseViewer::resizeSelf()
// {
//     isMaximized = !isMaximized;
//     emit resizeSplitter(this);
// }

void BaseViewer::onResizeButtonClicked()
{
    emit resizeButtonClicked();
}

/**
 * @brief called whenever the widget is resized by
 * dragging the splitter handles. This is reimplementation
 * QFrame's resizeEvent function
 * 
 * @param event 
 */
void BaseViewer::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);
    emit onFrameResize();
}

/**
 * @brief A vtkButtonWidget overlay for opengl renderers
 * 
 * @param renderer this is the renderer that will render the button
 * @param it vtkRenderWindowInteractor for the vtkButtonRepresentation 
 */
// void BaseViewer::addVtkResizeButton(vtkRenderer* renderer,
//     vtkRenderWindowInteractor* it)
// {
//     // overlay resize button
//     m_vtkResizeButton = vtkSmartPointer<VtkOverlayButton>::New();
//     m_vtkResizeButton->setupButton(renderer, it, ButtonPosition::RightFirstCol,
//         ButtonType::Resize, 30);

//     renderer->AddViewProp(m_vtkResizeButton->GetRepresentation());
//     m_vtkResizeButton->AddObserver(vtkCommand::StateChangedEvent, this,
//         &BaseViewer::onResizeButtonClicked);
//     m_vtkResizeButton->AddObserver(vtkCommand::StateChangedEvent, this,
//         &BaseViewer::resizeSelf);
//     renderer->GetRenderWindow()->AddObserver(vtkCommand::WindowResizeEvent,
//         m_vtkResizeButton, &VtkOverlayButton::repositionOnWindowResize);
// }

/**
* @brief Hide and disable the resize button
*/
// void BaseViewer::setVtkResizeButtonEnable(vtkRenderer* renderer, bool visible)
// {
//     if (!visible)
//     {
//         renderer->RemoveViewProp(m_vtkResizeButton->GetRepresentation());
//         m_vtkResizeButton->RemoveAllObservers();
//     }
//     else
//     {
//         renderer->AddViewProp(m_vtkResizeButton->GetRepresentation());
//         m_vtkResizeButton->AddObserver(vtkCommand::StateChangedEvent, this,
//             &BaseViewer::onResizeButtonClicked);
//         m_vtkResizeButton->AddObserver(vtkCommand::StateChangedEvent, this,
//             &BaseViewer::resizeSelf);
//     }
// }

// /**
//  * @brief A QPushButton overlay for QWidgets
//  * This also sets the icon and the icons size.
//  *
//  * @param parent pass the parent widget to overlay on
//  */
// void BaseViewer::addQResizeButton(QWidget* parent)
// {
//     m_qResizeButton = new QOverlayButton(parent, ButtonPosition::RightFirstCol,
//         ButtonType::Resize, 30);
//     m_qResizeButton->setToolTip(tr("Maximize"));

//     connect(this, &BaseViewer::onFrameResize, m_qResizeButton,
//         &QOverlayButton::repositionOnWindowResize);
//     connect(m_qResizeButton, &QOverlayButton::clicked, this,
//         &BaseViewer::resizeSelf);
// }

bool BaseViewer::IsMaximized()
{
    return isMaximized;
}

} // namespace visualization
}; // namespace ultrast