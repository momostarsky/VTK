set(test_exclusions)

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "fedora")
  list(APPEND test_exclusions
    # GPURayCast doesn't work with the CI's VNC setup.
    "TestGPURayCast"

    # These tests all seem to have some problem with the rendering order of
    # some components of the scenes that are being tested. Needs investigation.
    # https://gitlab.kitware.com/vtk/vtk/-/issues/18098
    "^VTK::CommonDataModelPython-TestHyperTreeGrid3DMandel$"
    "^VTK::FiltersCorePython-contourCells$"
    "^VTK::FiltersCorePython-contourQuadraticCells$"
    "^VTK::FiltersCorePython-TestPolyDataPlaneClipper2$"
    "^VTK::FiltersFlowPathsCxx-TestBSPTree$"
    "^VTK::FiltersGeneralCxx-TestDateToNumeric$"
    "^VTK::FiltersGeneralCxx-TestDensifyPolyData$"
    "^VTK::FiltersGeneralCxx-TestYoungsMaterialInterface$"
    "^VTK::FiltersGeneralPython-clipQuadraticCells$"
    "^VTK::FiltersGeneralPython-edgePoints$"
    "^VTK::FiltersGeneralPython-TestCellDerivs$"
    "^VTK::FiltersGeneralPython-TestDiscreteFlyingEdgesClipper2D$"
    "^VTK::FiltersGeneralPython-TestFEDiscreteClipper2D$"
    "^VTK::FiltersGeneralPython-TestSampleImplicitFunctionFilter$"
    "^VTK::FiltersGeometryCxx-TestExplicitStructuredGridSurfaceFilter$"
    "^VTK::FiltersGeometryCxx-TestLinearToQuadraticCellsFilter$"
    "^VTK::FiltersGeometryPython-LagrangeGeometricOperations$"
    "^VTK::FiltersHybridPython-depthSort$"
    "^VTK::FiltersHybridPython-imageToPolyData$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridBinaryEllipseMaterial$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridBinaryHyperbolicParaboloidMaterial$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridTernary3DAxisClipBox$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridTernary3DDualContour$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridTernary3DPlaneCutterDual$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridTernaryHyperbola$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridTernarySphereMaterial$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridTernarySphereMaterialReflections$"
    "^VTK::FiltersHyperTreeCxx-TestHyperTreeGridToDualGrid$"
    "^VTK::FiltersModelingCxx-TestQuadRotationalExtrusionMultiBlock$"
    "^VTK::FiltersModelingPython-TestCookieCutter$"
    "^VTK::FiltersModelingPython-TestCookieCutter3$"
    "^VTK::FiltersParallelDIY2Cxx-MPI-TestRedistributeDataSetFilterOnIoss$"
    "^VTK::FiltersPointsPython-TestConnectedPointsFilter$"
    "^VTK::FiltersPointsPython-TestFitImplicitFunction$"
    "^VTK::FiltersPointsPython-TestHierarchicalBinningFilter$"
    "^VTK::FiltersPointsPython-TestPCANormalEstimation$"
    "^VTK::FiltersPointsPython-TestPCANormalEstimation2$"
    "^VTK::FiltersPointsPython-TestRadiusOutlierRemoval$"
    "^VTK::FiltersPointsPython-TestSignedDistanceFilter$"
    "^VTK::FiltersPointsPython-TestVoxelGridFilter$"
    "^VTK::FiltersSourcesPython-TestStaticCellLocatorLineIntersection$"
    "^VTK::FiltersTexturePython-textureThreshold$"
    "^VTK::GeovisGDALCxx-TestRasterReprojectionFilter$"
    "^VTK::ImagingCorePython-Spectrum$"
    "^VTK::ImagingCorePython-TestMapToWindowLevelColors2$"
    "^VTK::InteractionWidgetsCxx-TestDijkstraImageGeodesicPath$"
    "^VTK::InteractionWidgetsCxx-TestPickingManagerWidgets$"
    "^VTK::InteractionWidgetsCxx-TestSeedWidget2$"
    "^VTK::InteractionWidgetsPython-TestPointCloudWidget$"
    "^VTK::InteractionWidgetsPython-TestPointCloudWidget2$"
    "^VTK::InteractionWidgetsPython-TestTensorWidget$"
    "^VTK::InteractionWidgetsPython-TestTensorWidget2$"
    "^VTK::IOGeometryPython-ParticleReader$"
    "^VTK::IOImageCxx-TestCompressedTIFFReader$"
    "^VTK::IOImageCxx-TestDICOMImageReader$"
    "^VTK::IOImageCxx-TestDICOMImageReaderFileCollection$"
    "^VTK::IOImageCxx-TestTIFFReaderMulti$"
    "^VTK::IOImportCxx-OBJImport-MixedOrder1$"
    "^VTK::IOImportCxx-OBJImport-MTLwithoutTextureFile$"
    "^VTK::IOIossCxx-MPI-TestIossExodusParitionedFiles$"
    "^VTK::IOIossCxx-MPI-TestIossExodusRestarts$"
    "^VTK::IOLASCxx-TestLASReader_test_1$"
    "^VTK::IOLASCxx-TestLASReader_test_2$"
    "^VTK::IOPDALCxx-TestPDALReader_test_1$"
    "^VTK::IOPDALCxx-TestPDALReader_test_2$"
    "^VTK::RenderingAnnotationCxx-TestCornerAnnotation$"
    "^VTK::RenderingCoreCxx-TestEdgeFlags$"
    "^VTK::RenderingCoreCxx-TestTextureRGBA$"
    "^VTK::RenderingCoreCxx-TestTextureRGBADepthPeeling$"
    "^VTK::RenderingCorePython-PickerWithLocator$"
    "^VTK::RenderingExternalCxx-TestGLUTRenderWindow$"
    "^VTK::RenderingFreeTypeCxx-TestFontDPIScaling$"
    "^VTK::RenderingFreeTypeCxx-TestFreeTypeTextMapper$"
    "^VTK::RenderingFreeTypeCxx-TestFreeTypeTextMapperWithColumns$"
    "^VTK::RenderingFreeTypeCxx-TestMathTextFreeTypeTextRenderer$"
    "^VTK::RenderingImagePython-TestDepthImageToPointCloud$"
    "^VTK::RenderingOpenGL2Cxx-TestCameraShiftScale$"
    "^VTK::RenderingOpenGL2Cxx-TestCoincident$"
    "^VTK::RenderingOpenGL2Cxx-TestCompositePolyDataMapper2CameraShiftScale$"
    "^VTK::RenderingOpenGL2Cxx-TestCompositePolyDataMapper2CellScalars$"
    "^VTK::RenderingOpenGL2Python-TestTopologyResolution$"
    "^VTK::RenderingVolumeCxx-TestRemoveVolumeNonCurrentContext$"
    "^VTKExample-Medical/Cxx$")
endif ()

string(REPLACE ";" "|" test_exclusions "${test_exclusions}")
if (test_exclusions)
  set(test_exclusions "(${test_exclusions})")
endif ()
