/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Visitors
#include "Visitors/NearestNeighborsDefaultVisitor.hpp"

// Descriptor visitors
#include "Visitors/DescriptorVisitors/ImagePatchDescriptorVisitor.hpp"

// Inpainting visitors
#include "Visitors/InpaintingVisitor.hpp"
#include "Visitors/AcceptanceVisitors/DefaultAcceptanceVisitor.hpp"

// Nearest neighbors functions
#include "NearestNeighbor/LinearSearchBestHistogram.hpp"
#include "NearestNeighbor/LinearSearchKNNProperty.hpp"
#include "NearestNeighbor/TwoStepNearestNeighbor.hpp"

// Initializers
#include "Initializers/InitializeFromMaskImage.hpp"
#include "Initializers/InitializePriority.hpp"

// Inpainters
#include "Inpainters/MaskImagePatchInpainter.hpp"
#include "Inpainters/HoleListPatchInpainter.hpp"

// Difference functions
#include "DifferenceFunctions/ImagePatchDifference.hpp"
#include "DifferenceFunctions/SumAbsolutePixelDifference.hpp"
#include "DifferenceFunctions/SumSquaredPixelDifference.hpp"

// Utilities
#include "Utilities/PatchHelpers.h"

// Inpainting
#include "Algorithms/InpaintingAlgorithm.hpp"

// Priority
#include "Priority/PriorityRandom.h"
#include "Priority/PriorityCriminisi.h"

// ITK
#include "itkImageFileReader.h"

// Submodules
#include <Helpers/Helpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>

// Run with: Data/trashcan.png Data/trashcan.mask 15 filled.png
int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 6)
  {
    std::cerr << "Required arguments: image.png imageMask.mask patchHalfWidth numberOfKNN output.png" << std::endl;
    std::cerr << "Input arguments: ";
    for(int i = 1; i < argc; ++i)
    {
      std::cerr << argv[i] << " ";
    }
    return EXIT_FAILURE;
  }

  std::stringstream ssArguments;
  for(int i = 1; i < argc; ++i)
  {
    ssArguments << argv[i] << " ";
    std::cout << argv[i] << " ";
  }

  // Parse arguments
  std::string imageFilename;
  std::string maskFilename;

  unsigned int patchHalfWidth = 0;

  unsigned int numberOfKNN = 0;

  std::string outputFilename;

  ssArguments >> imageFilename >> maskFilename >> patchHalfWidth >> numberOfKNN >> outputFilename;

  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch half width: " << patchHalfWidth << std::endl;
  std::cout << "numberOfKNN: " << numberOfKNN << std::endl;
  std::cout << "Output: " << outputFilename << std::endl;

//  typedef itk::VectorImage<float, 2> FloatVectorImageType;
//  typedef FloatVectorImageType ImageType;

  typedef itk::VectorImage<unsigned char, 2> InputImageType;

  typedef  itk::ImageFileReader<InputImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

//  ImageType::Pointer image = ImageType::New();
//  ITKHelpers::DeepCopy(imageReader->GetOutput(), image.GetPointer());

  // Convert the image to HSV
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  //ITKVTKHelpers::ConvertRGBtoHSV(image.GetPointer(), hsvImage.GetPointer());
  ITKVTKHelpers::ConvertRGBtoHSV(imageReader->GetOutput(), image.GetPointer());

  ITKHelpers::WriteImage(image.GetPointer(), "HSVImage.mha");

  Mask::Pointer mask = Mask::New();
  mask->Read(maskFilename);

  std::cout << "hole pixels: " << mask->CountHolePixels() << std::endl;
  std::cout << "valid pixels: " << mask->CountValidPixels() << std::endl;

  // Check the mask for compatability (if it gets too close to the image boundary, the algorithm breaks).
//  bool compatibleMask = PatchHelpers::CheckSurroundingRegionsOfAllHolePixels(mask, patchHalfWidth);
//  if(!compatibleMask)
//  {
//    throw std::runtime_error("The mask is not compatible!");
//  }

  typedef ImagePatchPixelDescriptor<ImageType> ImagePatchPixelDescriptorType;

  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = { { image->GetLargestPossibleRegion().GetSize()[0],
                                                      image->GetLargestPossibleRegion().GetSize()[1] } };
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  // Create the priority map
  typedef boost::vector_property_map<float, IndexMapType> PriorityMapType;
  PriorityMapType priorityMap(num_vertices(graph), indexMap);

  // Create the boundary status map. A node is on the current boundary if this property is true.
  // This property helps the boundaryNodeQueue because we can mark here if a node has become no longer
  // part of the boundary, so when the queue is popped we can check this property to see if it should
  // actually be processed.
  typedef boost::vector_property_map<bool, IndexMapType> BoundaryStatusMapType;
  BoundaryStatusMapType boundaryStatusMap(num_vertices(graph), indexMap);

  // Create the descriptor map. This is where the data for each pixel is stored.
  typedef boost::vector_property_map<ImagePatchPixelDescriptorType, IndexMapType> ImagePatchDescriptorMapType;
  ImagePatchDescriptorMapType imagePatchDescriptorMap(num_vertices(graph), indexMap);

  // Create the patch inpainter. The inpainter needs to know the status of each pixel to determine if they should be inpainted.
  typedef MaskImagePatchInpainter InpainterType;
  InpainterType patchInpainter(patchHalfWidth, mask);

  // Create the priority function
//  typedef PriorityRandom PriorityType;
//  bool random = false;
//  PriorityType priorityFunction(random);

//  typedef PriorityCriminisi<ImageType> PriorityType;
//  PriorityType priorityFunction(image, mask, patchHalfWidth);

  typedef PriorityOnionPeel PriorityType;
  PriorityType priorityFunction(mask, patchHalfWidth);

  // Create the boundary node queue. The priority of each node is used to order the queue.
  typedef boost::vector_property_map<std::size_t, IndexMapType> IndexInHeapMap;
  IndexInHeapMap index_in_heap(indexMap);

  // Create the priority compare functor (we want to use the highest priority pixels first, so the std::greater functor sorts the queue
  // such that the highest values (highest priorities) are first in the queue)
  typedef std::greater<float> PriorityCompareType;
  PriorityCompareType queueSortFunctor;

  typedef boost::d_ary_heap_indirect<VertexDescriptorType, 4, IndexInHeapMap, PriorityMapType, PriorityCompareType>
      BoundaryNodeQueueType;
  BoundaryNodeQueueType boundaryNodeQueue(priorityMap, index_in_heap, queueSortFunctor);

  // Create the descriptor visitor
  typedef ImagePatchDescriptorVisitor<VertexListGraphType, ImageType, ImagePatchDescriptorMapType>
      ImagePatchDescriptorVisitorType;
  ImagePatchDescriptorVisitorType imagePatchDescriptorVisitor(image, mask, imagePatchDescriptorMap, patchHalfWidth);

  typedef DefaultAcceptanceVisitor<VertexListGraphType> AcceptanceVisitorType;
  AcceptanceVisitorType acceptanceVisitor;

  // Create the inpainting visitor
  typedef InpaintingVisitor<VertexListGraphType, ImageType, BoundaryNodeQueueType,
      ImagePatchDescriptorVisitorType, AcceptanceVisitorType, PriorityType,
      PriorityMapType, BoundaryStatusMapType>
      InpaintingVisitorType;
  InpaintingVisitorType inpaintingVisitor(image, mask, boundaryNodeQueue,
                                          imagePatchDescriptorVisitor, acceptanceVisitor, priorityMap,
                                          &priorityFunction, patchHalfWidth,
                                          boundaryStatusMap, outputFilename);

  InitializePriority(mask, boundaryNodeQueue, priorityMap, &priorityFunction, boundaryStatusMap);

  // Initialize the boundary node queue from the user provided mask image.
  InitializeFromMaskImage<InpaintingVisitorType, VertexDescriptorType>(mask, &inpaintingVisitor);
  std::cout << "PatchBasedInpaintingNonInteractive: There are " << boundaryNodeQueue.size()
            << " nodes in the boundaryNodeQueue" << std::endl;

//  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumAbsolutePixelDifference<ImageType::PixelType> > PatchDifferenceType;
  typedef ImagePatchDifference<ImagePatchPixelDescriptorType, SumSquaredPixelDifference<ImageType::PixelType> > PatchDifferenceType;

  // Create the  neighbor finder
  typedef LinearSearchKNNProperty<ImagePatchDescriptorMapType, PatchDifferenceType> KNNSearchType;
  KNNSearchType linearSearchKNN(imagePatchDescriptorMap, numberOfKNN);

  typedef LinearSearchBestHistogram<ImagePatchDescriptorMapType, ImageType> BestSearchType;
  BestSearchType linearSearchBest(imagePatchDescriptorMap, image.GetPointer(), mask);
  linearSearchBest.SetNumberOfBinsPerDimension(30);

  TwoStepNearestNeighbor<KNNSearchType, BestSearchType> twoStepNearestNeighbor(linearSearchKNN, linearSearchBest);

  // Perform the inpainting
  InpaintingAlgorithm(graph, inpaintingVisitor, &boundaryStatusMap, &boundaryNodeQueue, twoStepNearestNeighbor, patchInpainter);

  return EXIT_SUCCESS;
}
