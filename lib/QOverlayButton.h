/**
 * @file QOverlayButton.h
 * @author Alvee Ahmed
 * @brief Dervied from QPushButton.
 * These overlay buttons are meant to be overlaid on 
 * QWidgets.
 * 
 * @version 0.1
 * @date 2021-09-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include "BaseOverlayButton.h"
#include <QPushButton>
#include <QFrame>


namespace ultrast {
namespace visualization {

class QOverlayButton: public QPushButton, public BaseOverlayButton {
    Q_OBJECT

    public:
        QOverlayButton(QWidget* parent, ButtonPosition position, ButtonType type, int size, bool checkable = false);
        ~QOverlayButton();

    private:
        QWidget* m_parent;

    public slots:
        void repositionOnWindowResize();
        void changeIcon(bool checked);




};

}
}