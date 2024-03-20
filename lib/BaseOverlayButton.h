#pragma once

#include <QString>
#include <QSize>
#include <QPointF>

namespace ultrast {
namespace visualization {

enum ButtonPosition {
    RightFirstCol = 0,
    RightSecondCol,
    RightThirdCol,
    RightFourthCol,
    RightFifthCol, 
    RightSixthCol,
    RightSeventhCol,
    RightEigthCol,
};

enum ViewerWindowType : unsigned int
{
    SagitalWindow = 0,
    AxialWindow = 1,
    CoronalWindow = 2
};

enum ButtonType {
    Resize = 0,
    Menu,
    Export,
    ImageInfo,
    TransducerCrosshair,
    IgtlSend,
    Zoom,
    ResizeRawImage,
    MeasureLength,
    MeasureAngle,
    DrawEllipse,
    DrawRectangle,
    DrawTrace,
    ShowMeasurement,
    Axis,
    Trash,
    Close,
    GridLines
};

enum ButtonAxisSubType : unsigned int {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    CLOCKWISE_ROTATE = 4,
    ANTICLOCKWISE_ROTATE = 5,
    SIZE_VERTICAL_INCREASE = 6,
    SIZE_VERTICAL_DECREASE = 7,
    SIZE_HORIZONTAL_DECREASE = 8,
    SIZE_HORIZONTAL_INCREASE = 9
};

class BaseOverlayButton {

    public:
        BaseOverlayButton(ButtonPosition position, ButtonType type, double size);
        BaseOverlayButton();
        ~BaseOverlayButton();

    protected:
        void setIconResource();

        double m_size;
        int m_space;
        ButtonPosition m_position;
        ButtonType m_type;
        QString m_iconResource;
        QString m_iconResourceNotChecked;
};



class BaseCustomizedButton {

public:
    BaseCustomizedButton(QSize btnSize, ButtonType type);
    BaseCustomizedButton() {};
    ~BaseCustomizedButton() {};
    void SetMainType(ButtonType type) { m_type = type; };
    void SetBtnPos(QPointF pos) { m_buttonPosOnWindow = pos; };
    int GetSubType() { return m_subType; };
    QPointF GetBtnPos() { return m_buttonPosOnWindow; };
    void setIconResource();
protected:
    QSize m_buttonSize;
    QPointF m_buttonPosOnWindow;
    ButtonType m_type; // primary property
    ButtonAxisSubType m_subType; // subset property, like the index or sequence or define by yourself
    QString m_iconResource;
    QString m_iconResourceNotChecked;
};

}
}