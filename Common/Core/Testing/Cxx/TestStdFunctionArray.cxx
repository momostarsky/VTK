/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestStdFunctionArray.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStdFunctionArray.h"

#include "vtkArrayDispatch.h"
#include "vtkDataArrayRange.h"
#include "vtkIntArray.h"

#include <cstdlib>
#include <memory>

namespace
{

struct ScaleWorker
{
  template <typename SrcArray, typename DstArray>
  void operator()(SrcArray* srcArr, DstArray* dstArr, double scale)
  {
    using SrcType = vtk::GetAPIType<SrcArray>;
    using DstType = vtk::GetAPIType<DstArray>;

    const auto srcRange = vtk::DataArrayValueRange(srcArr);
    auto dstRange = vtk::DataArrayValueRange(dstArr);

    if (srcRange.size() != dstRange.size())
    {
      std::cout << "Different array sizes in ScaleWorker" << std::endl;
      return;
    }

    auto dstIter = dstRange.begin();
    for (SrcType srcVal : srcRange)
    {
      *dstIter++ = static_cast<DstType>(srcVal * scale);
    }
  }
};

};

int TestStdFunctionArray(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  int res = EXIT_SUCCESS;

  vtkNew<vtkStdFunctionArray<int>> identity;
  auto identity_func = [](int idx) { return idx; };
  identity->SetBackend(std::make_shared<std::function<int(int)>>(identity_func));
  identity->SetNumberOfTuples(100);
  identity->SetNumberOfComponents(1);

  {
    for (int iArr = 0; iArr < 100; iArr++)
    {
      if (identity->GetValue(iArr) != iArr)
      {
        res = EXIT_FAILURE;
        std::cout << "get value failed with vtkStdFunctionArray" << std::endl;
      }
    }
  }

  {
    int iArr = 0;
    for (auto val : vtk::DataArrayValueRange<1>(identity))
    {
      if (val != iArr)
      {
        res = EXIT_FAILURE;
        std::cout << "range iterator failed with vtkStdFunctionArray" << std::endl;
      }
      iArr++;
    }
  }

#ifdef DISPATCH_STD_FUNCTION_ARRAYS
  {
    std::cout << "vtkStdFunctionArray: performing dispatch tests" << std::endl;
    vtkNew<vtkIntArray> destination;
    destination->SetNumberOfTuples(100);
    destination->SetNumberOfComponents(1);
    if (!vtkArrayDispatch::Dispatch2::Execute(identity, destination, ::ScaleWorker(), 3.0))
    {
      res = EXIT_FAILURE;
      std::cout << "vtkArrayDispatch failed with vtkStdFunctionArray" << std::endl;
    }
    int iArr = 0;
    for (auto val : vtk::DataArrayValueRange<1>(destination))
    {
      if (val != 3 * iArr)
      {
        res = EXIT_FAILURE;
        std::cout << "dispatch failed to populate the array with the correct values" << std::endl;
      }
      iArr++;
    }
  }
#endif // DISPATCH_STD_FUNCTION_ARRAYS
  return res;
};
