/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef ImageCollection_H
#define ImageCollection_H

#include "Mask.h"

// This class stores a vector of images of any type, and has some operations defined that can be applied to the whole list.

class ImageCollection
{
public:
  void AddImage(itk::ImageBase<2>* image);
  void Clear();

  // Apply Helpers::CopySelfPatchIntoHoleOfTargetRegion to all images in the collection.
  void CopySelfPatchIntoHoleOfTargetRegion(const Mask::Pointer mask, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion);

protected:
  std::vector<itk::ImageBase<2>*> Images;

};

#endif
