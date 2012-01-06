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

class vtkImageData;
class vtkPolyData;

namespace VTKHelpers
{

void GetCellCenter(vtkImageData* imageData, const unsigned int cellId, double center[3]);

// Set the center pixel of an 'image' to the specified 'color'. The image is assumed to have odd dimensions.
void SetImageCenterPixel(vtkImageData* image, const unsigned char color[3]);


// Set an image to black except for its border, which is set to 'color'.
void BlankAndOutlineImage(vtkImageData*, const unsigned char color[3]);

// Set an image to black.
void BlankImage(vtkImageData*);


// Extract the non-zero pixels of a "vector image" and convert them to vectors in a vtkPolyData.
// This is useful because glyphing a vector image is too slow to use as a visualization,
// because it "draws" the vectors, even if they are zero length. In this code we are often
// interested in displaying vectors along a contour, so this is a very very small subset of a whole vector image.
void KeepNonZeroVectors(vtkImageData* const image, vtkPolyData* output);

// Make pixels where the 0th channel of inputImage matches 'value' transparent in the output image.
void MakeValueTransparent(vtkImageData* const inputImage, vtkImageData* outputImage, const unsigned char value);

// Make an entire image transparent.
void MakeImageTransparent(vtkImageData* image);


} // end namespace
