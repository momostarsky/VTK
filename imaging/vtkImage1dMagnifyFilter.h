/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImage1dMagnifyFilter.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkImage1dMagnifyFilter - Magnifies an image.
// .SECTION Description
// vtkImage1dMagnifyFilter maps each pixel of the input onto a n (integer)
// region of the output.  Location (0,0) remains in the same place.
// The filter can use pixel replication (nearest neighbor) or interpolation.


#ifndef __vtkImage1dMagnifyFilter_h
#define __vtkImage1dMagnifyFilter_h


#include "vtkImageFilter.h"

class vtkImage1dMagnifyFilter : public vtkImageFilter
{
public:
  vtkImage1dMagnifyFilter();
  char *GetClassName() {return "vtkImage1dMagnifyFilter";};

  // Description:
  // Set/Get the convolution axis.
  vtkSetMacro(MagnificationFactor,int);
  vtkGetMacro(MagnificationFactor,int);

  // Description:
  // Pixel replication or Interpolation.
  vtkSetMacro(Interpolate,int);
  vtkGetMacro(Interpolate,int);
  vtkBooleanMacro(Interpolate,int);
  
  void InterceptCacheUpdate(vtkImageRegion *region);

protected:
  int MagnificationFactor;
  int Interpolate;

  void ComputeOutputImageInformation(vtkImageRegion *inRegion,
				     vtkImageRegion *outRegion);
  void ComputeRequiredInputRegionBounds(vtkImageRegion *outRegion,
					vtkImageRegion *inRegion);
  void Execute2d(vtkImageRegion *inRegion, vtkImageRegion *outRegion);  
};

#endif



