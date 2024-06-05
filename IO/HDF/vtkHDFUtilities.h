// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @namespace vtkHDFUtilities
 * @brief Common utility variables and functions for reader and writer of vtkHDF
 *
 */

#ifndef vtkHDFUtilities_h
#define vtkHDFUtilities_h

#include "vtkDataArray.h"
#include "vtkHDF5ScopedHandle.h"
#include "vtkIOHDFModule.h" // For export macro
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkType.h"

#include <array>
#include <string>
#include <vector>

VTK_ABI_NAMESPACE_BEGIN

namespace vtkHDFUtilities
{
const std::string VTKHDF_ROOT_PATH = "/VTKHDF";

/*
 * The number of PolyData topologies saved in vtkHDF format
 */
constexpr std::size_t NUM_POLY_DATA_TOPOS = 4;

/*
 * A vector of the topology names that are saved in vtkHDF
 * Can be used for the name of the HDF group only
 */
const std::vector<std::string> POLY_DATA_TOPOS{ "Vertices", "Lines", "Polygons", "Strips" };

/*
 * Attribute tag used in the cache storage to indicate arrays related to the geometry of the data
 * set and not fields of the data set
 */
constexpr int GEOMETRY_ATTRIBUTE_TAG = -42;

/*
 * How many attribute types we have. This returns 3: point, cell and field
 * attribute types.
 */
constexpr static int GetNumberOfAttributeTypes()
{
  return 3;
}

/*
 * Returns the id to a HDF datatype (H5T) from a VTK datatype
 * Returns H5I_INVALID_HID if no corresponding type is found
 */
inline hid_t getH5TypeFromVtkType(const int dataType)
{
  switch (dataType)
  {
    case VTK_DOUBLE:
      return H5T_NATIVE_DOUBLE;

    case VTK_FLOAT:
      return H5T_NATIVE_FLOAT;

#if VTK_ID_TYPE_IMPL == VTK_LONG_LONG
    case VTK_ID_TYPE:
#endif
    case VTK_LONG_LONG:
      return H5T_NATIVE_LLONG;

    case VTK_UNSIGNED_LONG_LONG:
      return H5T_NATIVE_ULLONG;

#if VTK_ID_TYPE_IMPL == VTK_LONG
    case VTK_ID_TYPE:
#endif
    case VTK_LONG:
      return H5T_NATIVE_LONG;

    case VTK_UNSIGNED_LONG:
      return H5T_NATIVE_ULONG;

#if VTK_ID_TYPE_IMPL == VTK_INT
    case VTK_ID_TYPE:
#endif
    case VTK_INT:
      return H5T_NATIVE_INT;

    case VTK_UNSIGNED_INT:
      return H5T_NATIVE_UINT;

    case VTK_SHORT:
      return H5T_NATIVE_SHORT;

    case VTK_UNSIGNED_SHORT:
      return H5T_NATIVE_USHORT;

    case VTK_CHAR:
      return H5T_NATIVE_CHAR;

    case VTK_SIGNED_CHAR:
      return H5T_NATIVE_SCHAR;

    case VTK_UNSIGNED_CHAR:
      return H5T_NATIVE_UCHAR;

    default:
      return H5I_INVALID_HID;
  }
}

VTK_DEPRECATED_IN_9_4_0("Please use TemporalGeometryOffsets struct instead.")
struct VTKIOHDF_EXPORT TransientGeometryOffsets
{
public:
  bool Success = true;
  vtkIdType PartOffset = 0;
  vtkIdType PointOffset = 0;
  std::vector<vtkIdType> CellOffsets;
  std::vector<vtkIdType> ConnectivityOffsets;

  template <class T>
  TransientGeometryOffsets(T* impl, vtkIdType step)
  {
    auto recupMultiOffset = [&](std::string path, std::vector<vtkIdType>& val) {
      val = impl->GetMetadata(path.c_str(), 1, step);
      if (val.empty())
      {
        vtkErrorWithObjectMacro(
          nullptr, << path.c_str() << " array cannot be empty when there is temporal data");
        return false;
      }
      return true;
    };
    auto recupSingleOffset = [&](std::string path, vtkIdType& val) {
      std::vector<vtkIdType> buffer;
      if (!recupMultiOffset(path, buffer))
      {
        return false;
      }
      val = buffer[0];
      return true;
    };
    if (!recupSingleOffset("Steps/PartOffsets", this->PartOffset))
    {
      this->Success = false;
      return;
    }
    if (!recupSingleOffset("Steps/PointOffsets", this->PointOffset))
    {
      this->Success = false;
      return;
    }
    if (!recupMultiOffset("Steps/CellOffsets", this->CellOffsets))
    {
      this->Success = false;
      return;
    }
    if (!recupMultiOffset("Steps/ConnectivityIdOffsets", this->ConnectivityOffsets))
    {
      this->Success = false;
      return;
    }
  }
};

/*
 * @struct TemporalGeometryOffsets
 * @brief Use to get the offsets for temporal vtkHDF.
 *
 * To use it, create an object using the templated constructor of this struct.
 * It will fill the object with data that can be then retrieved.
 */
struct VTKIOHDF_EXPORT TemporalGeometryOffsets
{
public:
  bool Success = true;
  vtkIdType PartOffset = 0;
  vtkIdType PointOffset = 0;
  std::vector<vtkIdType> CellOffsets;
  std::vector<vtkIdType> ConnectivityOffsets;

  template <class T>
  TemporalGeometryOffsets(T* impl, vtkIdType step)
  {
    auto recupMultiOffset = [&](std::string path, std::vector<vtkIdType>& val) {
      val = impl->GetMetadata(path.c_str(), 1, step);
      if (val.empty())
      {
        vtkErrorWithObjectMacro(
          nullptr, << path.c_str() << " array cannot be empty when there is temporal data");
        return false;
      }
      return true;
    };
    auto recupSingleOffset = [&](std::string path, vtkIdType& val) {
      std::vector<vtkIdType> buffer;
      if (!recupMultiOffset(path, buffer))
      {
        return false;
      }
      val = buffer[0];
      return true;
    };
    if (!recupSingleOffset("Steps/PartOffsets", this->PartOffset))
    {
      this->Success = false;
      return;
    }
    if (!recupSingleOffset("Steps/PointOffsets", this->PointOffset))
    {
      this->Success = false;
      return;
    }
    if (!recupMultiOffset("Steps/CellOffsets", this->CellOffsets))
    {
      this->Success = false;
      return;
    }
    if (!recupMultiOffset("Steps/ConnectivityIdOffsets", this->ConnectivityOffsets))
    {
      this->Success = false;
      return;
    }
  }
};

/**
 * Open a VTK HDF file and checks if it is valid.
 * On succeed fileID is set to a valid hid.
 */
VTKIOHDF_EXPORT bool Open(const char* fileName, hid_t& fileID);

/**
 * Convert C++ template type T to HDF5 native type
 * this can be constexpr in C++17 standard
 */
template <typename T>
VTKIOHDF_EXPORT hid_t TemplateTypeToHdfNativeType();

/**
 * Reads an attribute from the group passed to it
 */
template <typename T>
VTKIOHDF_EXPORT bool GetAttribute(
  hid_t group, const char* attributeName, size_t numberOfElements, T* value);

/**
 * Read the number of steps of an HDF group
 */
VTKIOHDF_EXPORT std::size_t GetNumberOfSteps(hid_t groupID);

/**
 * Returns the dimensions of a HDF dataset of a file.
 */
VTKIOHDF_EXPORT std::vector<hsize_t> GetDimensions(hid_t fileID, const char* datasetName);

/**
 * Initialize meta information of the file.
 */
VTKIOHDF_EXPORT bool RetrieveHDFInformation(hid_t& fileID, hid_t& groupID,
  const std::string& rootName, std::array<int, 2>& version, int& dataSetType, int& numberOfPieces,
  std::array<hid_t, 3>& attributeDataGroup);

/**
 * Convenient callback method to retrieve a name when calling a H5Giterate()
 */
VTKIOHDF_EXPORT herr_t FileInfoCallBack(
  hid_t loc_id, const char* name, const H5L_info_t* info, void* opdata);

/**
 * Returns the names of arrays for 'attributeType' (point or cell).
 */
VTKIOHDF_EXPORT std::vector<std::string> GetArrayNames(
  const std::array<hid_t, 3>& attributeDataGroup, int attributeType);

/**
 * Return the name of all children of an HDF group given its path
 */
VTKIOHDF_EXPORT std::vector<std::string> GetOrderedChildrenOfGroup(
  hid_t groupID, const std::string& path);

/**
 * Opens the hdf5 dataset given the 'group' and 'name'.
 * Returns the hdf dataset and sets 'nativeType' and 'dims'.
 * The caller needs to close the returned hid_t manually using H5Dclose or a Scoped Handle if it
 * is not an invalid hid.
 */
VTKIOHDF_EXPORT hid_t OpenDataSet(
  hid_t group, const char* name, hid_t* nativeType, std::vector<hsize_t>& dims);

///@{
/**
 * Reads a vtkDataArray of type T from the attributeType, dataset
 * The array has type 'T' and 'numberOfComponents'. We are reading
 * fileExtent slab from the array. It returns the array or nullptr
 * in case of an error.
 * There are three cases for fileExtent:
 * fileExtent.size() == 0 - in this case we expect a 1D array and we read
 *                          the whole array. Used for field arrays.
 * fileExtent.size()>>1 == ndims - in this case we read a scalar
 * fileExtent.size()>>1 + 1 == ndims - in this case we read an array with
 *                           the number of components > 1.
 */
VTKIOHDF_EXPORT vtkDataArray* NewArrayForGroup(hid_t dataset, hid_t nativeType,
  const std::vector<hsize_t>& dims, const std::vector<hsize_t>& parameterExtent);
VTKIOHDF_EXPORT vtkDataArray* NewArrayForGroup(
  hid_t group, const char* name, const std::vector<hsize_t>& parameterExtent);
///@}

/**
 * Reads a 1D metadata array in a DataArray or a vector of vtkIdType.
 * We read either the whole array for the vector version or a slice
 * specified with (offset, size). For an error we return nullptr or an
 * empty vector.
 */
VTKIOHDF_EXPORT std::vector<vtkIdType> GetMetadata(
  hid_t group, const char* name, hsize_t size, hsize_t offset);

/**
 * Return the field array size (components, tuples) for the HDF group.
 * By default it returns {-1,1} which means to have as many components as necessary
 * and one tuple per step.
 */
VTKIOHDF_EXPORT std::array<vtkIdType, 2> GetFieldArraySize(
  hid_t group, vtkIdType step, std::string name);

/**
 * Methods to query for array offsets for the HDF group when steps are present
 */
VTKIOHDF_EXPORT vtkIdType GetArrayOffset(
  hid_t group, vtkIdType step, int attributeType, std::string name);

/**
 * Reads and returns a new vtkAbstractArray. The actual type of the array
 * depends on the type of the HDF array. The array is read from the PointData
 * or CellData groups depending on the 'attributeType' parameter.
 */
VTKIOHDF_EXPORT vtkAbstractArray* NewFieldArray(const std::array<hid_t, 3>& attributeDataGroup,
  const char* name, vtkIdType offset, vtkIdType size, vtkIdType dimMaxSize);
}

VTK_ABI_NAMESPACE_END
#include "vtkHDFUtilities.txx" // for template implementations

#endif
// VTK-HeaderTest-Exclude: vtkHDFUtilities.h
