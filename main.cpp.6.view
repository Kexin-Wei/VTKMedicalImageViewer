#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <QVTKOpenGLWidget.h>
#include <QVTKOpenGLWindow.h>
#include <QWidget>

#include <vtkSmartPointer.h>

// #include <src/QuadQtVTKRenderWidget.h>
#include <src/VTKOpenGLWidget.h>
// #include "QVTKOpenGLNativeWidget.h"

int main(int argc, char* argv[])
{
    QSurfaceFormat::setDefaultFormat(VTKOpenGLWidget::defaultFormat());

    QApplication app(argc, argv);
    // QMainWindow w;
    // QuadQtVTKRenderWidget* renderWidget = new QuadQtVTKRenderWidget(&w);
    // w.setCentralWidget(renderWidget);
    // w.show();
    VTKOpenGLWidget widget;
    widget.show();

    return app.exec();
}
