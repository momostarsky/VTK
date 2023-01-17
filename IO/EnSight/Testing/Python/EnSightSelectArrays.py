#!/usr/bin/env python
from vtkmodules.vtkCommonExecutionModel import vtkCompositeDataPipeline
from vtkmodules.vtkFiltersGeometry import vtkGeometryFilter
from vtkmodules.vtkIOEnSight import vtkGenericEnSightReader
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkHierarchicalPolyDataMapper,
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkRenderer,
)
import vtkmodules.vtkInteractionStyle
import vtkmodules.vtkRenderingFreeType
import vtkmodules.vtkRenderingOpenGL2
from vtkmodules.util.misc import vtkGetDataRoot
VTK_DATA_ROOT = vtkGetDataRoot()

# create a rendering window and renderer
ren1 = vtkRenderer()
renWin = vtkRenderWindow()
renWin.AddRenderer(ren1)
renWin.StereoCapableWindowOn()
iren = vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)
reader = vtkGenericEnSightReader()
# Make sure all algorithms use the composite data pipeline
cdp = vtkCompositeDataPipeline()
reader.SetDefaultExecutivePrototype(cdp)
reader.SetCaseFileName("" + str(VTK_DATA_ROOT) + "/Data/EnSight/blow1_ascii.case")
reader.SetTimeValue(1)
reader.ReadAllVariablesOff()
reader.SetPointArrayStatus("displacement",1)
reader.SetCellArrayStatus("thickness",1)
reader.SetCellArrayStatus("displacement",1)
geom = vtkGeometryFilter()
geom.SetInputConnection(reader.GetOutputPort())
mapper = vtkHierarchicalPolyDataMapper()
mapper.SetInputConnection(geom.GetOutputPort())
mapper.SetScalarRange(0.5,1.0)
actor = vtkActor()
actor.SetMapper(mapper)
# assign our actor to the renderer
ren1.AddActor(actor)
# enable user interface interactor
iren.Initialize()
ren1.GetActiveCamera().SetPosition(99.3932,17.6571,-22.6071)
ren1.GetActiveCamera().SetFocalPoint(3.5,12,1.5)
ren1.GetActiveCamera().SetViewAngle(30)
ren1.GetActiveCamera().SetViewUp(0.239617,-0.01054,0.97081)
ren1.ResetCameraClippingRange()
renWin.Render()
# prevent the tk window from showing up then start the event loop
reader.SetDefaultExecutivePrototype(None)
# --- end of script --
