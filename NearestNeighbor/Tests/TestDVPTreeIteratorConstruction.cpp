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

// Pixel descriptors
#include "PixelDescriptors/ImagePatchPixelDescriptor.h"

// Visitors
//#include "Visitors/DefaultInpaintingVisitor.hpp"
#include "Visitors/RandomDescriptorCreatorVisitor.hpp"

// Nearest neighbors
#include "NearestNeighbor/metric_space_search.hpp"

// Initializers
#include "Initializers/SimpleInitializer.hpp"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/graph/grid_graph.hpp>
#include <boost/property_map/property_map.hpp>

namespace boost
{
  enum vertex_data_t { vertex_data };
  BOOST_INSTALL_PROPERTY(vertex, data);
};

int main(int argc, char *argv[])
{
  // Create the graph
  typedef boost::grid_graph<2> VertexListGraphType;
  boost::array<std::size_t, 2> graphSideLengths = {{300, 300}};
  VertexListGraphType graph(graphSideLengths);
  typedef boost::graph_traits<VertexListGraphType>::vertex_descriptor VertexDescriptorType;

  // Create the topology
  const unsigned int descriptorDimension = 100;
  typedef boost::hypercube_topology<descriptorDimension, boost::minstd_rand> TopologyType;
  TopologyType space;

  // Get the index map
  typedef boost::property_map<VertexListGraphType, boost::vertex_index_t>::const_type IndexMapType;
  IndexMapType indexMap(get(boost::vertex_index, graph));

  // Create the descriptor map. This is where the data for each pixel is stored. The Topology
  typedef boost::vector_property_map<TopologyType::point_type, IndexMapType> DescriptorMapType;
  DescriptorMapType descriptorMap(num_vertices(graph), indexMap);

  //DefaultInpaintingVisitor visitor;
  RandomDescriptorCreatorVisitor<DescriptorMapType> visitor(descriptorMap, descriptorDimension);

  // Initialize
  SimpleInitializer(&visitor, graph);

  // Create the tree
  std::cout << "Creating tree..." << std::endl;
  typedef dvp_tree<VertexDescriptorType, TopologyType, DescriptorMapType > TreeType;
  // TreeType tree(graph, space, descriptorMap); // Create from all positions on the graph_traits - don't want to do this unless all positions on the topology are valid.

  // std::vector<TopologyType::point_type> validPositions; // Don't store the positions, but rather store the vertex_descriptors
  std::vector<VertexDescriptorType> validPositions;
  typename boost::graph_traits<VertexListGraphType>::vertex_iterator vi,vi_end;

  for (tie(vi,vi_end) = vertices(graph); vi != vi_end; ++vi)
  {
    validPositions.push_back(*vi);
  }

  std::cout << "There are " << validPositions.size() << " validPositions." << std::endl;

  std::cout << "Creating tree..." << std::endl;
  TreeType tree(validPositions.begin(), validPositions.end(), space, descriptorMap);
  std::cout << "Finished creating tree." << std::endl;

  // Create the nearest neighbor finder
  typedef multi_dvp_tree_search<VertexListGraphType, TreeType> SearchType;
  SearchType nearestNeighborFinder;
  nearestNeighborFinder.graph_tree_map[&graph] = &tree;

  return EXIT_SUCCESS;
}
