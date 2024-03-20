/**
 * @file QOverlayButton.cpp
 * @author Alvee Ahmed
 * @brief QOverlayButton is derived from QPushButton to be overlaid on QWidgets
 * @version 0.1
 * @date 2021-09-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <QOverlayButton.h>
// #include <infrastructure\utility\Logger.h>

#include <QPalette>

// using Logger = ultrast::infrastructure::utility::Logger;

namespace ultrast {
namespace visualization {

QOverlayButton::QOverlayButton(QWidget* parent, ButtonPosition position,
    ButtonType type, int size, bool checkable) :
    QPushButton(parent),
    BaseOverlayButton(position, type, size),
    m_parent(parent)
{
    this->setStyleSheet("QPushButton {background : transparent;}");
    this->setCursor(QCursor(Qt::PointingHandCursor));
    this->setIconSize(QSize(m_size, m_size));
    this->setMaximumSize(QSize(m_size, m_size));
    this->setIcon(QIcon(m_iconResource));
    this->setCheckable(checkable);
    this->raise();

    if (checkable)
        connect(this, &QOverlayButton::clicked, this,
            &QOverlayButton::changeIcon);
}

QOverlayButton::~QOverlayButton()
{
}

void QOverlayButton::changeIcon(bool checked)
{
    if (checked)
        this->setIcon(QIcon(m_iconResourceNotChecked));
    else
        this->setIcon(QIcon(m_iconResource));
}

/**
 * @brief QSlot for repositioning the overlay button whenever the window is resized
 * 
 */
void QOverlayButton::repositionOnWindowResize()
{
    int x = m_parent->size().width()
        - (this->size().width() * (m_position + 1) * 1.2);
    int y = 0 * this->size().height();

    this->move(x, y);
}

}
}