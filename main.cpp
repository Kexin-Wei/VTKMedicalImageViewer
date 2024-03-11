#include "QVTKOpenGLNativeWidget.h"
#include <QApplication>
#include <QMainWindow>
#include <src/QuadQtVTKRenderWidget.h>
#include <src/VTKOpenGLWidget.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    // QMainWindow w;
    // QuadQtVTKRenderWidget* renderWidget = new QuadQtVTKRenderWidget(&w);
    // w.setCentralWidget(renderWidget);
    // w.show();
    VTKOpenGLWidget widget;
    widget.show();
    return app.exec();
}
