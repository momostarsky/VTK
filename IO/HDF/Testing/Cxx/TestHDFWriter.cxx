// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkHDFReader.h"
#include "vtkHDFWriter.h"
#include "vtkImageData.h"
#include "vtkLogger.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPartitionedDataSet.h"
#include "vtkPartitionedDataSetCollection.h"
#include "vtkPassArrays.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"
#include "vtkTestUtilities.h"
#include "vtkTesting.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkXMLMultiBlockDataReader.h"
#include "vtkXMLPartitionedDataSetCollectionReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLUnstructuredGridReader.h"

#include "vtkHDF5ScopedHandle.h"
#include "vtk_hdf5.h"

#include <string>

namespace
{
struct WriterConfigOptions
{
  bool UseExternalPartitions;
  bool UseExternalComposite;
  bool MergePartsOnRead; // Should be false when reading PartitionedData
  std::string FileNameSuffix;
  int CompressionLevel;
};
}
//----------------------------------------------------------------------------
bool WriteMiscData(const std::string& filename)
{
  // Appending groups alongside "VTKHDF" in the file should not alter how the reader behaves
  vtkHDF::ScopedH5FHandle file{ H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT) };
  if (file == H5I_INVALID_HID)
  {
    std::cerr << "Could not re-open " << filename << file << " for writing";
    return false;
  }

  // Create groups
  vtkHDF::ScopedH5GHandle misc{ H5Gcreate(file, "Misc", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) };
  vtkHDF::ScopedH5GHandle misc2{ H5Gcreate(file, "VTKHD", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) };

  // Create more groups inside
  vtkHDF::ScopedH5GHandle data1{ H5Gcreate(misc, "Data1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) };
  vtkHDF::ScopedH5GHandle data2{ H5Gcreate(misc, "Data2", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) };
  vtkHDF::ScopedH5GHandle data3{ H5Gcreate(data1, "Data3", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) };

  // Create dataspace
  vtkHDF::ScopedH5SHandle dataspace{ H5Screate(H5S_SIMPLE) };
  std::array<hsize_t, 1> dimensions{ 3 };
  H5Sset_extent_simple(dataspace, 1, dimensions.data(), dimensions.data());

  // Create and fill dataset
  vtkHDF::ScopedH5DHandle dataset = H5Dcreate(
    data3, "MiscDataset", H5T_STD_I64LE, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  std::array<int, 3> values{ 4, 5, 3 };
  H5Dwrite(dataset, H5T_STD_I64LE, H5S_ALL, dataspace, H5P_DEFAULT, values.data());

  return true;
}

//----------------------------------------------------------------------------
bool TestEmptyPolyData(const std::string& tempDir)
{
  std::string filePath = tempDir + "/emptyPolyData.vtkhdf";
  vtkNew<vtkPolyData> pd;
  vtkNew<vtkHDFWriter> writer;
  writer->SetInputData(pd);
  writer->SetFileName(filePath.c_str());
  writer->Write();
  return true;
}

//----------------------------------------------------------------------------
bool TestWriteAndRead(
  vtkDataObject* data, const std::string& tempPath, WriterConfigOptions* options = nullptr)
{
  std::string fullPath = tempPath;
  vtkNew<vtkHDFWriter> writer;
  writer->SetInputData(data);
  if (options)
  {
    fullPath = tempPath + options->FileNameSuffix;
    writer->SetFileName(fullPath.c_str());
    writer->SetUseExternalComposite(options->UseExternalComposite);
    writer->SetUseExternalPartitions(options->UseExternalPartitions);
    writer->SetCompressionLevel(options->CompressionLevel);

    vtkLog(INFO,
      "Testing " << fullPath << " with options Ext composite: " << options->UseExternalComposite
                 << " ext partitions: " << options->UseExternalPartitions << " compression "
                 << options->CompressionLevel);
  }
  else
  {
    writer->SetFileName(fullPath.c_str());
  }

  writer->Write();

  // Append data that should be ignored by the reader
  if (!WriteMiscData(fullPath))
  {
    return false;
  }

  vtkNew<vtkHDFReader> reader;
  if (!reader->CanReadFile(fullPath.c_str()))
  {
    std::cerr << "vtkHDFReader can not read file: " << fullPath << std::endl;
    return false;
  }
  reader->SetFileName(fullPath.c_str());
  if (options)
  {
    reader->SetMergeParts(options->MergePartsOnRead);
  }
  reader->Update();
  vtkDataObject* output = vtkDataObject::SafeDownCast(reader->GetOutput());
  if (output == nullptr)
  {
    std::cerr << "vtkHDFReader does not output a vtkDataObject when reading: " << tempPath
              << std::endl;
    return false;
  }

  if (!vtkTestUtilities::CompareDataObjects(output, data))
  {
    std::cerr << "vtkDataObject does not match: " << tempPath << std::endl;
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool TestWriteAndReadConfigurations(vtkDataObject* data, const std::string& path, bool mergeParts)
{
  std::vector<WriterConfigOptions> options{ { false, false, mergeParts, "_NoExtPartNoExtComp", 3 },
    { false, true, mergeParts, "_NoExtPartExtComp", 1 },
    { true, true, mergeParts, "_ExtPartExtComp", 2 },
    { true, false, mergeParts, "_ExtPartNoExtComp", 5 } };

  for (auto& optionSet : options)
  {
    if (!TestWriteAndRead(data, path, &optionSet))
    {
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
bool TestSpherePolyData(const std::string& tempDir)
{
  vtkNew<vtkSphereSource> sphere;
  sphere->SetThetaResolution(100);
  sphere->SetPhiResolution(100);
  sphere->SetRadius(1);
  sphere->Update();
  vtkPolyData* spherePd = sphere->GetOutput();

  std::string filePath = tempDir + "/spherePolyData.vtkhdf";
  return TestWriteAndRead(spherePd, filePath);
}

//----------------------------------------------------------------------------
bool TestComplexPolyData(const std::string& tempDir, const std::string& dataRoot)
{
  const std::vector<std::string> baseNames = { "cow.vtp", "isofill_0.vtp" };
  for (const auto& baseName : baseNames)
  {
    // Get a polydata from a VTP
    const std::string basePath = dataRoot + "/Data/" + baseName;
    vtkNew<vtkXMLPolyDataReader> baseReader;
    baseReader->SetFileName(basePath.c_str());
    baseReader->Update();
    vtkPolyData* baseData = vtkPolyData::SafeDownCast(baseReader->GetOutput());
    if (baseData == nullptr)
    {
      std::cerr << "Can't read base data from: " << basePath << std::endl;
      return false;
    }

    // Write and read the polydata in a temp file, compare with base
    std::string tempPath = tempDir + "/HDFWriter_" + baseName + ".vtkhdf";
    if (!TestWriteAndRead(baseData, tempPath))
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool TestUnstructuredGrid(const std::string& tempDir, const std::string& dataRoot)
{
  std::vector<std::string> baseNames = { "explicitStructuredGrid.vtu",
    "explicitStructuredGridEmpty.vtu", "elements.vtu" };
  for (const auto& baseName : baseNames)
  {
    // Get an Unstructured grid from a VTU
    const std::string basePath = dataRoot + "/Data/" + baseName;
    vtkNew<vtkXMLUnstructuredGridReader> baseReader;
    baseReader->SetFileName(basePath.c_str());
    baseReader->Update();
    vtkUnstructuredGrid* baseData = vtkUnstructuredGrid::SafeDownCast(baseReader->GetOutput());
    if (baseData == nullptr)
    {
      std::cerr << "Can't read base data from: " << basePath << std::endl;
      return false;
    }

    // Write and read the unstructuredGrid in a temp file, compare with base
    std::string tempPath = tempDir + "/HDFWriter_" + baseName + ".vtkhdf";
    if (!TestWriteAndRead(baseData, tempPath))
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool TestPartitionedUnstructuredGrid(const std::string& tempDir, const std::string& dataRoot)
{
  std::vector<std::string> baseNames = { "can-pvtu.hdf" };
  for (const auto& baseName : baseNames)
  {
    // Get an Partitioned Unstructured grid from a VTKHDF file
    const std::string basePath = dataRoot + "/Data/" + baseName;
    vtkNew<vtkHDFReader> baseReader;
    baseReader->SetFileName(basePath.c_str());
    baseReader->SetMergeParts(false);
    baseReader->Update();
    auto baseData = vtkPartitionedDataSet::SafeDownCast(baseReader->GetOutput());
    if (baseData == nullptr)
    {
      std::cerr << "Can't read base data from: " << basePath << std::endl;
      return false;
    }

    // Write and read the partitioned unstructuredGrid in a temp file, compare with base
    std::string tempPath = tempDir + "/HDFWriter_" + baseName + ".vtkhdf";
    if (!TestWriteAndReadConfigurations(baseData, tempPath, false))
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool TestPartitionedPolyData(const std::string& tempDir, const std::string& dataRoot)
{
  std::vector<std::string> baseNames = { "test_poly_data.hdf" };
  for (const auto& baseName : baseNames)
  {
    // Get an Partitioned PolyData from a VTKHDF file
    const std::string basePath = dataRoot + "/Data/" + baseName;
    vtkNew<vtkHDFReader> baseReader;
    baseReader->SetFileName(basePath.c_str());
    baseReader->SetMergeParts(false);
    baseReader->Update();
    auto baseData = vtkPartitionedDataSet::SafeDownCast(baseReader->GetOutput());
    if (baseData == nullptr)
    {
      std::cerr << "Can't read base data from: " << basePath << std::endl;
      return false;
    }

    // Write and read the partitioned PolyData in a temp file, compare with base
    std::string tempPath = tempDir + "/HDFWriter_" + baseName + ".vtkhdf";
    if (!TestWriteAndReadConfigurations(baseData, tempPath, false))
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool TestMultiBlock(const std::string& tempDir, const std::string& dataRoot)
{
  std::vector<std::string> baseNamesMB = { "test_multiblock_hdf.vtm" };
  for (const auto& baseName : baseNamesMB)
  {
    // Read the multiblock from vtm file
    const std::string basePath = dataRoot + "/Data/vtkHDF/" + baseName;
    vtkNew<vtkXMLMultiBlockDataReader> baseReader;
    baseReader->SetFileName(basePath.c_str());
    baseReader->Update();
    vtkMultiBlockDataSet* baseData = vtkMultiBlockDataSet::SafeDownCast(baseReader->GetOutput());
    if (baseData == nullptr)
    {
      std::cerr << "Can't read base data from: " << basePath << std::endl;
      return false;
    }

    // Write and read the vtkMultiBlockDataSet in a temp file, compare with base
    std::string tempPath = tempDir + "/HDFWriter_" + baseName + ".vtkhdf";
    if (!TestWriteAndReadConfigurations(baseData, tempPath, true))
    {
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
bool TestPartitionedDataSetCollection(const std::string& tempDir, const std::string& dataRoot)
{
  std::vector<std::string> baseNamesMB = { "dummy_pdc_structure.vtpc", "multi_ds_pdc.vtpc" };
  for (const auto& baseName : baseNamesMB)
  {
    // Get a PDC from a vtpc file
    const std::string basePath = dataRoot + "/Data/vtkHDF/" + baseName;
    vtkNew<vtkXMLPartitionedDataSetCollectionReader> baseReader;
    baseReader->SetFileName(basePath.c_str());
    baseReader->Update();
    vtkPartitionedDataSetCollection* baseData =
      vtkPartitionedDataSetCollection::SafeDownCast(baseReader->GetOutput());
    if (baseData == nullptr)
    {
      std::cerr << "Can't read base data from: " << basePath << std::endl;
      return false;
    }

    // Write and read the vtkPartitionedDataSetCollection in a temp file, compare with base
    std::string tempPath = tempDir + "/HDFWriter_" + baseName + ".vtkhdf";
    if (!TestWriteAndReadConfigurations(baseData, tempPath, true))
    {
      return false;
    }
  }

  return true;
}

#include "vtkHDF5ScopedHandle.h"
#include <array>

//----------------------------------------------------------------------------
int TestHDFWriter(int argc, char* argv[])
{
  // Get temporary testing directory
  char* tempDirCStr =
    vtkTestUtilities::GetArgOrEnvOrDefault("-T", argc, argv, "VTK_TEMP_DIR", "Testing/Temporary");
  std::string tempDir{ tempDirCStr };
  delete[] tempDirCStr;

  // Get data directory
  // vtkNew<vtkTesting> testHelper;
  // testHelper->AddArguments(argc, argv);
  // if (!testHelper->IsFlagSpecified("-D"))
  // {
  //   std::cerr << "Error: -D /path/to/data was not specified." << std::endl;
  //   return EXIT_FAILURE;
  // }
  // std::string dataRoot = testHelper->GetDataRoot();

  // // Run tests
  // bool testPasses = true;
  // testPasses &= TestEmptyPolyData(tempDir);
  // testPasses &= TestSpherePolyData(tempDir);
  // testPasses &= TestComplexPolyData(tempDir, dataRoot);
  // testPasses &= TestUnstructuredGrid(tempDir, dataRoot);
  // testPasses &= TestPartitionedUnstructuredGrid(tempDir, dataRoot);
  // testPasses &= TestPartitionedPolyData(tempDir, dataRoot);
  // testPasses &= TestPartitionedDataSetCollection(tempDir, dataRoot);
  // testPasses &= TestMultiBlock(tempDir, dataRoot);

  // {
  //   // Create subfile
  //   std::string filename = "test_0.hdf";
  //   vtkHDF::ScopedH5FHandle file =
  //     H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  //   // Create a 60*3 chunked dataset in subfile
  //   vtkHDF::ScopedH5PHandle plist = H5Pcreate(H5P_DATASET_CREATE);
  //   H5Pset_layout(plist, H5D_CHUNKED);
  //   hsize_t SMALL_CHUNK[] = { 1, 1 };
  //   H5Pset_chunk(plist, 2, SMALL_CHUNK);
  //   std::vector<hsize_t> dims{ 60, 3 };
  //   std::vector<hsize_t> max_dims{ H5S_UNLIMITED, 3 };
  //   vtkHDF::ScopedH5SHandle dataspace{ H5Screate_simple(2, dims.data(), max_dims.data()) };
  //   vtkHDF::ScopedH5DHandle dataset =
  //     H5Dcreate(file, "data", H5T_IEEE_F32LE, dataspace, H5P_DEFAULT, plist, H5P_DEFAULT);
  //   std::vector<float> data(60 * 3, 2.0f);
  //   data[0] = 5.0f;
  //   data[120] = 3.0f;
  //   data[121] = 4.0f;
  //   H5Dwrite(dataset, H5T_IEEE_F32LE, H5S_ALL, dataspace, H5P_DEFAULT, data.data());

  //   /***********/

  //   // Create a new file referencing the one we just created through a virtual dataset
  //   std::string mainFilename = "test.hdf";
  //   vtkHDF::ScopedH5FHandle mainFile =
  //     H5Fcreate(mainFilename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  //   // This will contain the mappings
  //   vtkHDF::ScopedH5PHandle virtualSourceP = H5Pcreate(H5P_DATASET_CREATE);

  //   // Create destination space
  //   std::vector<hsize_t> dspaceDims{ 40, 3 };
  //   vtkHDF::ScopedH5SHandle destSpace = H5Screate_simple(2, dspaceDims.data(), nullptr);

  //   // Create mapping dataspace
  //   std::vector<hsize_t> mappingDims{ 20, 3 };
  //   vtkHDF::ScopedH5SHandle mappedDataSpace = H5Screate_simple(2, dims.data(), nullptr);

  //   // Select source & destination hyperslab, build mapping
  //   std::vector<hsize_t> offsetSource{ 0, 0 };
  //   std::vector<hsize_t> offsetDestination{ 0, 0 };
  //   H5Sselect_hyperslab(
  //     destSpace, H5S_SELECT_SET, offsetSource.data(), nullptr, mappingDims.data(), nullptr);
  //   H5Sselect_hyperslab(mappedDataSpace, H5S_SELECT_SET, offsetDestination.data(), nullptr,
  //     mappingDims.data(), nullptr);
  //   H5Pset_virtual(virtualSourceP, destSpace, "test_0.hdf", "data", mappedDataSpace);

  //   // Create a second mapping
  //   offsetSource[0] = 20;
  //   offsetDestination[0] = 40;
  //   H5Sselect_hyperslab(
  //     destSpace, H5S_SELECT_SET, offsetSource.data(), nullptr, mappingDims.data(), nullptr);
  //   H5Sselect_hyperslab(mappedDataSpace, H5S_SELECT_SET, offsetDestination.data(), nullptr,
  //     mappingDims.data(), nullptr);
  //   H5Pset_virtual(virtualSourceP, destSpace, "test_0.hdf", "data", mappedDataSpace);

  //   // Create dataset
  //   vtkHDF::ScopedH5DHandle vdset = H5Dcreate(
  //     mainFile, "composite", H5T_IEEE_F32LE, destSpace, H5P_DEFAULT, virtualSourceP,
  //     H5P_DEFAULT);
  // }

  // {
  //   std::string filename = "test.hdf";

  //   // Open Testing/Temporary/parallel_time_cow.vtkhdf
  //   vtkHDF::ScopedH5FHandle fileID = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

  //   // Open dataset PointData/Normals
  //   vtkHDF::ScopedH5DHandle dataset = H5Dopen(fileID, "composite", H5P_DEFAULT);
  //   hid_t nativeType = H5Dget_type(dataset);

  //   // Select 20x3 hyperslab in source dataspace
  //   std::vector<hsize_t> count{ 20, 3 };
  //   std::vector<hsize_t> offset{ 0, 0 };
  //   vtkHDF::ScopedH5SHandle dataspace = H5Dget_space(dataset);
  //   H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.data(), nullptr, count.data(),
  //   nullptr);

  //   // Create destination memory space
  //   vtkHDF::ScopedH5SHandle memspace =
  //     H5Screate_simple(static_cast<int>(count.size()), count.data(), nullptr);

  //   // Select hyperslab in destination space
  //   H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset.data(), nullptr, count.data(), nullptr);

  //   // Read the hyperslab
  //   std::array<std::array<float, 3>, 20> data;
  //   H5Dread(dataset, nativeType, memspace, dataspace, H5P_DEFAULT, data.data());
  // }

  int nbRanks = 3;
  for (int myRank = 0; myRank < 3; myRank++)
  {
    std::string filePath = tempDir + "/parallel_time_cow.vtkhdf";
    std::string filePathPart =
      tempDir + "/parallel_time_cow_part" + std::to_string(myRank) + ".vtkhdf";

    vtkNew<vtkHDFReader> reader;
    reader->SetFileName(filePath.c_str());
    reader->UpdatePiece(myRank, nbRanks, 0);

    vtkNew<vtkHDFReader> readerPart;
    readerPart->SetFileName(filePathPart.c_str());
    readerPart->Update();

    const std::vector<double> timeValues{ 1.0, 3.0, 5.0 };
    for (int time = 0; time < timeValues.size(); time++)
    {
      vtkDebugWithObjectMacro(nullptr, << "Comparing timestep " << time);
      reader->SetStep(time);
      reader->UpdatePiece(myRank, nbRanks, 0);
      readerPart->SetStep(time);
      readerPart->Update();
      // pass->UpdateTimeStep(time);
      // pass->Update();

      vtkUnstructuredGrid* readPiece =
        vtkUnstructuredGrid::SafeDownCast(reader->GetOutputDataObject(0));
      vtkNew<vtkPassArrays> pass2;
      pass2->SetRemoveArrays(true);
      pass2->AddArray(vtkDataObject::FIELD, "Time");
      pass2->SetInputDataObject(readPiece);
      pass2->Update();
      readPiece = vtkUnstructuredGrid::SafeDownCast(pass2->GetOutputDataObject(0));
      // vtkUnstructuredGrid* originalPiece =
      //   vtkUnstructuredGrid::SafeDownCast(pass->GetOutputDataObject(0));
      vtkUnstructuredGrid* readPart =
        vtkUnstructuredGrid::SafeDownCast(readerPart->GetOutputDataObject(0));

      // if (readPiece == nullptr || originalPiece == nullptr || readPart == nullptr)
      // {
      //   vtkLog(ERROR, "Piece should not be null");
      //   return false;
      // }

      // if (!vtkTestUtilities::CompareDataObjects(readPiece, originalPiece))
      // {
      //   vtkLog(ERROR, "Original and read piece do not match");
      //   return false;
      // }

      if (!vtkTestUtilities::CompareDataObjects(readPiece, readPart))
      {
        vtkLog(ERROR, "Read piece and read part do not match");
        return false;
      }
    }
  }

  return EXIT_SUCCESS;
  // return testPasses ? EXIT_SUCCESS : EXIT_FAILURE;
}
