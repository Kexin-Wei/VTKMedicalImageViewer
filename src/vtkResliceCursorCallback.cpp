//
// Created by weike on 3/5/2024.
//

#include "vtkResliceCursorCallback.h"

#include <vtkPlaneSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>

vtkResliceCursorCallback* vtkResliceCursorCallback::New()
{
    return new vtkResliceCursorCallback;
}

void vtkResliceCursorCallback::Execute(vtkObject* caller,
    const unsigned long ev, void* callData)
{
    if (ev == vtkResliceCursorWidget::WindowLevelEvent
        || ev == vtkCommand::WindowLevelEvent
        || ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
    {
        // Render everything
        for (const auto& i : this->RCW)
        {
            i->Render();
        }
        this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
        return;
    }

    const vtkImagePlaneWidget* ipw = dynamic_cast<vtkImagePlaneWidget*>(caller);
    if (ipw)
    {
        const double* wl = static_cast<double*>(callData);

        if (ipw == this->IPW[0])
        {
            this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
            this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
        }
        else if (ipw == this->IPW[1])
        {
            this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
            this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
        }
        else if (ipw == this->IPW[2])
        {
            this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
            this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
        }
    }

    auto* rcw = dynamic_cast<vtkResliceCursorWidget*>(caller);
    if (rcw)
    {
        const auto rep = dynamic_cast<vtkResliceCursorLineRepresentation*>(
            rcw->GetRepresentation());
        // Although the return value is not used, we keep the get calls
        // in case they had side-effects
        rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
        for (int i = 0; i < 3; i++)
        {
            auto* ps = dynamic_cast<vtkPlaneSource*>(
                this->IPW[i]->GetPolyDataAlgorithm());
            ps->SetOrigin(this->RCW[i]
                              ->GetResliceCursorRepresentation()
                              ->GetPlaneSource()
                              ->GetOrigin());
            ps->SetPoint1(this->RCW[i]
                              ->GetResliceCursorRepresentation()
                              ->GetPlaneSource()
                              ->GetPoint1());
            ps->SetPoint2(this->RCW[i]
                              ->GetResliceCursorRepresentation()
                              ->GetPlaneSource()
                              ->GetPoint2());

            // If the reslice plane has modified, update it on the 3D widget
            this->IPW[i]->UpdatePlacement();
        }
    }

    // Render everything
    for (const auto& i : this->RCW)
    {
        i->Render();
    }
    this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
}
