// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
// This test verifies that we can have multiple render layers
//
// The command line arguments are:
// -I        => run in interactive mode; unless this is used, the program will
//              not allow interaction and exit

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkConeSource.h"
#include "vtkLogger.h"
#include "vtkNew.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"

#include "vtkAnariPass.h"
#include "vtkAnariRendererNode.h"
#include "vtkAnariTestUtilities.h"

int TestAnariLayers(int argc, char* argv[])
{
  vtkLogger::SetStderrVerbosity(vtkLogger::Verbosity::VERBOSITY_WARNING);
  bool useDebugDevice = false;

  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i], "-trace"))
    {
      useDebugDevice = true;
      vtkLogger::SetStderrVerbosity(vtkLogger::Verbosity::VERBOSITY_INFO);
    }
  }

  vtkNew<vtkRenderWindowInteractor> iren;
  vtkNew<vtkRenderWindow> renWin;
  iren->SetRenderWindow(renWin);
  renWin->SetNumberOfLayers(2);

  // Layer 0: OpenGL
  vtkNew<vtkRenderer> renderer;
  renWin->AddRenderer(renderer);
  vtkNew<vtkSphereSource> sphere;
  sphere->SetPhiResolution(10);
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(sphere->GetOutputPort());
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(0.0, 1.0, 0.0);
  renderer->AddActor(actor);
  renderer->SetBackground(0.5, 0.5, 1.0); // should see a light blue background

  // Layer 1: ANARI
  vtkNew<vtkRenderer> renderer2;
  renderer2->SetLayer(1);
  renWin->AddRenderer(renderer2);
  renderer2->SetBackground(1.0, 0.0, 0.0); // should not see red background
  vtkNew<vtkConeSource> cone;
  cone->SetResolution(100);
  // cone->SetDirection(0, 1, 0);
  cone->Update();
  vtkNew<vtkPolyDataMapper> mapper2;
  mapper2->SetInputConnection(cone->GetOutputPort());
  vtkNew<vtkActor> actor2;
  actor2->SetMapper(mapper2);
  actor2->GetProperty()->SetColor(0.53, 0.2, 0.0);
  actor2->GetProperty()->SetOpacity(0.5);
  renderer2->AddActor(actor2);

  vtkNew<vtkAnariPass> anariPass;
  renderer2->SetPass(anariPass);

  SetAnariRendererParameterDefaults(renderer2, useDebugDevice, "TestAnariLayers");

  renWin->SetSize(400, 400);
  renWin->Render();

  int retVal = vtkRegressionTestImageThreshold(renWin, 0.05);

  if (retVal == vtkRegressionTester::DO_INTERACTOR)
  {
    iren->Start();
  }

  return !retVal;
}
