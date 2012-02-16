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

#include "PatchBasedInpaintingViewerWidget.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QGraphicsPixmapItem>

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkImageSliceMapper.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLImageDataWriter.h> // For debugging only

// Custom
//#include "FileSelector.h"
#include "Helpers/Helpers.h"
#include "Helpers/HelpersOutput.h"
#include "HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"
#include "ImageProcessing/Mask.h"

PatchBasedInpaintingViewerWidget::PatchBasedInpaintingViewerWidget()
{
  // This function is called by both constructors. This avoid code duplication.
  this->setupUi(this);

  SetupScenes();

  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  this->UserImage = FloatVectorImageType::New();

  this->Camera = new ImageCamera(this->Renderer);

  this->UserMaskImage = Mask::New();

}

void PatchBasedInpaintingViewerWidget::SetupScenes()
{
  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(this->SceneBackground);

  this->TargetPatchScene = new QGraphicsScene();
  this->TargetPatchScene->setBackgroundBrush(brush);
  this->gfxTarget->setScene(TargetPatchScene);

  this->SourcePatchScene = new QGraphicsScene();
  this->SourcePatchScene->setBackgroundBrush(brush);
  this->gfxSource->setScene(SourcePatchScene);

  this->ResultPatchScene = new QGraphicsScene();
  this->ResultPatchScene->setBackgroundBrush(brush);
  this->gfxResult->setScene(ResultPatchScene);
}

void PatchBasedInpaintingViewerWidget::DisplaySourcePatch()
{
  // TODO: Fix this
//   QImage sourceImage = HelpersQt::GetQImage<FloatVectorImageType>(currentImage, this->SourcePatchToDisplay.Region, this->ImageDisplayStyle);
//   //sourceImage = HelpersQt::FitToGraphicsView(sourceImage, gfxSource);
//   QGraphicsPixmapItem* item = this->SourcePatchScene->addPixmap(QPixmap::fromImage(sourceImage));
//   gfxSource->fitInView(item);
//   LeaveFunction("DisplaySourcePatch()");
}

void PatchBasedInpaintingViewerWidget::DisplayTargetPatch()
{
  // We use the previous image and previous mask, but the current PotentialPairSets,
  // as these are the sets that were used to get to this state.

  // The last iteration record will not have any potential patches, because there is nothing left to inpaint!
//   if(!RecordToDisplay)
//     {
//     LeaveFunction("DisplayTargetPatch()");
//     return;
//     }
//   FloatVectorImageType::Pointer currentImage = dynamic_cast<FloatVectorImageType*>(this->RecordToDisplay->GetImageByName("Image").Image);

  // If we have chosen to display the masked target patch, we need to use the mask from the previous iteration
  // (as the current mask has been cleared where the target patch was copied).
  //Mask::Pointer currentMask = dynamic_cast<Mask*>(this->RecordToDisplay->Images.FindImageByName("Mask").Image.GetPointer());

  // Target
  // TODO: Fix this.
//   QImage targetImage = HelpersQt::GetQImage<FloatVectorImageType>(currentImage, this->TargetPatchToDisplay.GetRegion(), this->ImageDisplayStyle);
// 
//   //targetImage = HelpersQt::FitToGraphicsView(targetImage, gfxTarget);
//   QGraphicsPixmapItem* item = this->TargetPatchScene->addPixmap(QPixmap::fromImage(targetImage));
//   gfxTarget->fitInView(item);
}

void PatchBasedInpaintingViewerWidget::DisplayResultPatch()
{
  //QImage qimage(regionSize[0], regionSize[1], QImage::Format_RGB888);

  // TODO: Fix this
//   itk::ImageRegionIterator<FloatVectorImageType> sourceIterator(currentImage, this->SourcePatchToDisplay.GetRegion());
//   itk::ImageRegionIterator<FloatVectorImageType> targetIterator(currentImage, this->TargetPatchToDisplay.GetRegion());
//   itk::ImageRegionIterator<Mask> maskIterator(currentMask, this->TargetPatchToDisplay.GetRegion());
// 
//   FloatVectorImageType::Pointer resultPatch = FloatVectorImageType::New();
//   resultPatch->SetNumberOfComponentsPerPixel(currentImage->GetNumberOfComponentsPerPixel());
//   itk::Size<2> patchSize = ITKHelpers::SizeFromRadius(this->Settings.PatchRadius);
//   itk::ImageRegion<2> region;
//   region.SetSize(patchSize);
//   resultPatch->SetRegions(region);
//   resultPatch->Allocate();
// 
//   while(!maskIterator.IsAtEnd())
//     {
//     FloatVectorImageType::PixelType pixel;
// 
//     if(currentMask->IsHole(maskIterator.GetIndex()))
//       {
//       pixel = sourceIterator.Get();
//       }
//     else
//       {
//       pixel = targetIterator.Get();
//       }

//     itk::Offset<2> offset = sourceIterator.GetIndex() - this->SourcePatchToDisplay.GetRegion().GetIndex();
//     itk::Index<2> offsetIndex;
//     offsetIndex[0] = offset[0];
//     offsetIndex[1] = offset[1];
//     resultPatch->SetPixel(offsetIndex, pixel);

//     ++sourceIterator;
//     ++targetIterator;
//     ++maskIterator;
//     }

  // Color the center pixel
  //qimage.setPixel(regionSize[0]/2, regionSize[1]/2, this->CenterPixelColor.rgb());

//   qimage = HelpersQt::GetQImage<FloatVectorImageType>(resultPatch, resultPatch->GetLargestPossibleRegion(), this->ImageDisplayStyle);
// 
//   //qimage = HelpersQt::FitToGraphicsView(qimage, gfxResult);
//   this->ResultPatchScene->clear();
//   QGraphicsPixmapItem* item = this->ResultPatchScene->addPixmap(QPixmap::fromImage(qimage));
//   gfxResult->fitInView(item);
//   //this->ResultPatchScene->addPixmap(QPixmap());
//   //std::cout << "Set result patch." << std::endl;

}

/*
void PatchBasedInpaintingViewerWidget::IterationComplete(const PatchPair& usedPatchPair)
{
  // TODO: The interfaces used in this function have changed.

  InpaintingIterationRecord iterationRecord;
  for(unsigned int i = 0; i < this->Inpainting->GetImagesToUpdate().size(); ++i)
    {
    std::cout << "Updating image " << i << std::endl;

    ITKHelpers::OutputImageType(this->Inpainting->GetImagesToUpdate()[i]);
    itk::ImageBase<2>::Pointer newImage = ITKHelpers::CreateImageWithSameType(this->Inpainting->GetImagesToUpdate()[i]);
    ITKHelpers::OutputImageType(newImage);
    // TODO: Make this deep copy work
    //ITKHelpers::DeepCopy(this->Inpainting->GetImagesToUpdate()[i], newImage);
    }
  std::cout << "Finished creating record images." << std::endl;

}*/
