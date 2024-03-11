#include "QVTKOpenGLNativeWidget.h"
#include <QApplication>
#include <QMainWindow>
#include <src/QuadQtVTKRenderWidget.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QMainWindow w;
    QuadQtVTKRenderWidget* renderWidget = new QuadQtVTKRenderWidget(&w);
    w.setCentralWidget(renderWidget);
    w.show();
    return app.exec();
}
