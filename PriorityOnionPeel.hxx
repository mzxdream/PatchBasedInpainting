
#include "PriorityOnionPeel.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"

// VXL
//#include <vnl/vnl_double_2.h>

// ITK
#include "itkInvertIntensityImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// VTK
#include <vtkSmartPointer.h>

template <typename TImage>
PriorityOnionPeel<TImage>::PriorityOnionPeel(const TImage* const image, const Mask* const maskImage, unsigned int patchRadius) :
Priority<TImage>(image, maskImage, patchRadius)
{
  this->ConfidenceMapImage = FloatScalarImageType::New();
  InitializeConfidenceMap();
}

template <typename TImage>
FloatScalarImageType* PriorityOnionPeel<TImage>::GetConfidenceMapImage()
{
  return this->ConfidenceMapImage;
}

template <typename TImage>
std::vector<std::string> PriorityOnionPeel<TImage>::GetImageNames()
{
  std::vector<std::string> imageNames = Priority<TImage>::GetImageNames();
  imageNames.push_back("ConfidenceMap");
  return imageNames;
}

template <typename TImage>
std::vector<NamedVTKImage> PriorityOnionPeel<TImage>::GetNamedImages()
{
  std::vector<NamedVTKImage> namedImages = Priority<TImage>::GetNamedImages();

  NamedVTKImage confidenceMapImage;
  confidenceMapImage.Name = "ConfidenceMap";
  vtkSmartPointer<vtkImageData> confidenceMapImageVTK = vtkSmartPointer<vtkImageData>::New();
  ITKVTKHelpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->ConfidenceMapImage, confidenceMapImageVTK);
  confidenceMapImage.ImageData = confidenceMapImageVTK;
  namedImages.push_back(confidenceMapImage);

  return namedImages;
}

template <typename TImage>
void PriorityOnionPeel<TImage>::Update(const itk::ImageRegion<2>& filledRegion)
{
  //EnterFunction("PriorityOnionPeel::Update()");
  // Get the center pixel (the pixel around which the region was filled)
  itk::Index<2> centerPixel = ITKHelpers::GetRegionCenter(filledRegion);
  float value = ComputeConfidenceTerm(centerPixel);
  UpdateConfidences(filledRegion, value);

  this->MaskImage->FindBoundary(this->BoundaryImage);

  //LeaveFunction("PriorityOnionPeel::Update()");
}

template <typename TImage>
float PriorityOnionPeel<TImage>::ComputePriority(const itk::Index<2>& queryPixel)
{
  float priority = ComputeConfidenceTerm(queryPixel);

  return priority;
}

template <typename TImage>
void PriorityOnionPeel<TImage>::UpdateConfidences(const itk::ImageRegion<2>& targetRegion, const float value)
{
  //EnterFunction("PriorityOnionPeel::UpdateConfidences()");
  //std::cout << "Updating confidences with value " << value << std::endl;

  // Force the region to update to be entirely inside the image
  itk::ImageRegion<2> region = ITKHelpers::CropToRegion(targetRegion, this->Image->GetLargestPossibleRegion());

  // Use an iterator to find masked pixels. Compute their new value, and save it in a vector of pixels and their new values.
  // Do not update the pixels until after all new values have been computed, because we want to use the old values in all of
  // the computations.
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

  while(!maskIterator.IsAtEnd())
    {
    if(this->MaskImage->IsHole(maskIterator.GetIndex()))
      {
      itk::Index<2> currentPixel = maskIterator.GetIndex();
      this->ConfidenceMapImage->SetPixel(currentPixel, value);
      }

    ++maskIterator;
    } // end while looop with iterator
  //LeaveFunction("PriorityOnionPeel::UpdateConfidences()");

}

template <typename TImage>
void PriorityOnionPeel<TImage>::InitializeConfidenceMap()
{
  //EnterFunction("PriorityOnionPeel::InitializeConfidenceMap()");
  // Clone the mask - we need to invert the mask to actually perform the masking, but we don't want to disturb the original mask
  Mask::Pointer maskClone = Mask::New();
  //Helpers::DeepCopy<Mask>(this->CurrentMask, maskClone);
  maskClone->DeepCopyFrom(this->MaskImage);

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <Mask> InvertIntensityImageFilterType;
  InvertIntensityImageFilterType::Pointer invertIntensityFilter = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(maskClone);
  invertIntensityFilter->Update();

  // Convert the inverted mask to floats and scale them to between 0 and 1
  // to serve as the initial confidence image
  typedef itk::RescaleIntensityImageFilter< Mask, FloatScalarImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(invertIntensityFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();

  ITKHelpers::DeepCopy<FloatScalarImageType>(rescaleFilter->GetOutput(), this->ConfidenceMapImage);
  //LeaveFunction("PriorityOnionPeel::InitializeConfidenceMap()");
}

template <typename TImage>
float PriorityOnionPeel<TImage>::ComputeConfidenceTerm(const itk::Index<2>& queryPixel)
{
  //EnterFunction("PriorityOnionPeel::ComputeConfidenceTerm()");
  //DebugMessage<itk::Index<2>>("Computing confidence for ", queryPixel);

  // Allow for regions on/near the image border

  //itk::ImageRegion<2> region = this->CurrentMask->GetLargestPossibleRegion();
  //region.Crop(Helpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius[0]));
  itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);
  itk::ImageRegion<2> region = ITKHelpers::CropToRegion(targetRegion, this->Image->GetLargestPossibleRegion());

  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, region);

  // The confidence is computed as the sum of the confidences of patch pixels in the source region / area of the patch

  float sum = 0.0f;

  while(!maskIterator.IsAtEnd())
    {
    if(this->MaskImage->IsValid(maskIterator.GetIndex()))
      {
      sum += this->ConfidenceMapImage->GetPixel(maskIterator.GetIndex());
      }
    ++maskIterator;
    }

  unsigned int numberOfPixels = region.GetNumberOfPixels();
  float areaOfPatch = static_cast<float>(numberOfPixels);

  float confidence = sum/areaOfPatch;
  //DebugMessage<float>("Confidence: ", confidence);
  //LeaveFunction("PriorityOnionPeel::ComputeConfidenceTerm()");
  return confidence;

}