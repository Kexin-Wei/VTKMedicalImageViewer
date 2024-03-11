#pragma once
#ifndef VTKRESLICECURSORCALLBACK_H
#define VTKRESLICECURSORCALLBACK_H

#include <vtkCommand.h>
#include <vtkImagePlaneWidget.h>
#include <vtkResliceCursorWidget.h>

class vtkResliceCursorCallback : public vtkCommand
{
public:
    static vtkResliceCursorCallback* New();
    void Execute(vtkObject* caller, unsigned long ev,
        void* callData) override;
    vtkImagePlaneWidget* IPW[3];
    vtkResliceCursorWidget* RCW[3];
};

#endif // VTKRESLICECURSORCALLBACK_H
