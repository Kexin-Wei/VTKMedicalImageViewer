#include "QVTKOpenGLNativeWidget.h"
#include <QMainWindow>
#include <src/QuadQtVTKRenderWidget.h>

int main()
{
    QMainWindow w;
    QuadQtVTKRenderWidget* renderWidget = new QuadQtVTKRenderWidget(&w);
    w.setCentralWidget(renderWidget);
    w.show();
    return 1;
}
