#ifndef Inpainting_HPP
#define Inpainting_HPP

/**
 * This function template performs an in-painting of the hole-vertices of a graph.
 * This version performs an initialization of the vertices by calling upon the visitor 
 * to do so on all vertices. The visitor is responsible for correctly initializing the 
 * color value (black for holes, white otherwise), the priority value, and the 
 * position value, as well as any other values necessary for functors to function 
 * correctly.
 * 
 * \tparam VertexListGraph A graph type that can hold a list of vertices.
 * \tparam InpaintingVisitor A visitor type that models the InpaintingVisitorConcept which is used to inject all the custom code into this algorithm.
 * \tparam Topology A topology type that can compute a distance-value between two positions.
 * \tparam PositionMap A property-map type that can fetch the positions associated to vertices.
 * \tparam ColorMap A property-map type that can fetch the color-values associated to vertices.
 * \tparam PriorityMap A property-map type that can fetch the priority-values associated to vertices.
 * \tparam PriorityCompare A functor type that can compare priority-values (strict weak ordering).
 * \tparam NearestNeighborFinder A functor type that can find the nearest neighbor to a given vertex within a given topology.
 * \tparam PatchInpainter A functor type that traverse the holes in a patch and apply a visitor painting on them.
 * \param g A graph that holds all the vertices.
 * \param vis A visitor to inject all the custom code into this algorithm.
 * \param space A topology to compute a distance-value between two positions.
 * \param position A property-map to fetch the positions associated to vertices.
 * \param color A property-map to fetch the color-values associated to vertices.
 * \param priority A property-map to fetch the priority-values associated to vertices.
 * \param compare_priority A functor to compare priority-values (strict weak ordering).
 * \param find_inpainting_source A functor to find the nearest neighbor to a given vertex within a given topology.
 * \param inpaint_patch A functor to traverse the holes in a patch and apply a visitor's painting on them.
 */
template <typename VertexListGraph, typename InpaintingVisitor,
          typename Topology, typename PositionMap,
          typename ColorMap, typename PriorityMap, 
          typename PriorityCompare, 
          typename NearestNeighborFinder, 
          typename PatchInpainter>
inline
void inpainting(VertexListGraph& g, InpaintingVisitor vis,
                const Topology& space, PositionMap position,
                ColorMap color, PriorityMap priority, 
                PriorityCompare compare_priority,
                NearestNeighborFinder find_inpainting_source, 
                PatchInpainter inpaint_patch) {
  
  typedef typename boost::graph_traits<VertexListGraph>::vertex_descriptor Vertex;
  
  // initialize all vertices:
  {
    typename boost::graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = boost::vertices(g); ui != ui_end; ++ui) {
      vis.initialize_vertex(*ui,g);
    };
  };
  
  // start the in-painting:
  inpainting_no_init(g, vis, space, position, color, priority, compare_priority, 
                     find_inpainting_source, inpaint_patch);
  
};

#endif
