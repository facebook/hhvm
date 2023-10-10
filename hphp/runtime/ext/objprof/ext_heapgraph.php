<?hh

namespace HH {

/**
 * Return an integer describing the current format of stats, nodes,
 * and edges. This will be incremented when things change substantially
 * enough to require client detection
 */
function heapgraph_version(): int {
  return 2;
}

/**
 * Capture the current heapgraph
 *
 * @return resource - This is the heap graph resource. Use it with other
 *   heap graph functions to extract the information you want.
 */
<<__Native>>
function heapgraph_create(): resource;

/**
 * General statistics on the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 *
 * @return array<string, int> - General metrics describing the heap graph.
 * Keys are defined as follows:
 *
 *   nodes      count of nodes in the graph
 *   edges      count of pointers (edges) in the graph
 *   roots      count of root->nonroot pointers (pointers where from is root)
 *   root_nodes count of root nodes
 *   exact      1 if type scanners were built, 0 otherwise
 */
<<__Native>>
function heapgraph_stats(resource $heapgraph): darray<string, int>;

/**
 * Iterates over the nodes in the heap graph. Calls back on every node.
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param mixed $callback - function(array<string, mixed> $node): void {}
 * See documentation for heapgraph_node() about the "node" array passed
 * to $callback
 */
<<__Native>>
function heapgraph_foreach_node(resource $heapgraph, mixed $callback): void;

/**
 * Iterates over the root nodes in the heap graph. Calls back on every root
 * node. Out-edges from root nodes are also enumerable separately by calling
 * heapgraph_foreach_root().
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param mixed $callback - function(darray<string, mixed> $node): void {}
 * See documentation for heapgraph_node() about the "node" array passed
 * to $callback
 */
<<__Native>>
function heapgraph_foreach_root_node(resource $heapgraph,
                                     mixed $callback): void;

/**
 * Iterates over the edges in the heap graph. Calls back on every edge.
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param mixed $callback - function(darray<string, mixed> $edge): void {}
 * See documentation for heapgraph_edge() about the "edge" array passed
 * to $callback
 */
<<__Native>>
function heapgraph_foreach_edge(resource $heapgraph, mixed $callback): void;

/**
 * Iterates over the root edges in the heap graph. Calls back on every root
 * edge. The "from" field of each root edge will be root node. root nodes
 * are also enumerable by calling heapgraph_foreach_root_node().
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param mixed $callback - function(darray<string, mixed> $edge): void {}
 * See documentation for heapgraph_edge() about the "edge" array passed
 * to $callback
 */
<<__Native>>
function heapgraph_foreach_root(resource $heapgraph, mixed $callback): void;

/**
 * performs dfs over the heap graph, starting from the given nodes.
 * calls back on every new node in the scan.
 *
 * @param resource $heapgraph - the resource obtained with heapgraph_create
 * @param array<int> $roots - node indexes to start the scan from
 * @param array<int> $skips - node indexes to consider as if they're not there
 * @param mixed $callback - function(array<string, mixed> $node): void {}
 * See documentation for heapgraph_edge() about the "edge" array passed
 * to $callback
 */
<<__Native>>
function heapgraph_dfs_nodes(
  resource $heapgraph,
  varray<int> $roots,
  varray<int> $skips,
  mixed $callback,
): void;

/**
 * performs dfs over the heap graph, starting from the given edges.
 * calls back on every new edge in the scan.
 *
 * @param resource $heapgraph - the resource obtained with heapgraph_create
 * @param array<int> $roots - edge indexes to start the scan from
 * @param array<int> $skips - edge indexes to consider as if they're not there
 * @param mixed $callback -
 *  function(array<string, mixed> $edge, array<string, mixed> $node): void {}
 * See documentation for heapgraph_node() about the "node" array passed
 * to $callback
 */
<<__Native>>
function heapgraph_dfs_edges(
  resource $heapgraph,
  varray<int> $roots,
  varray<int> $skips,
  mixed $callback
): void;

/**
 * Get a specific node from the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param int $index - The node index
 *
 * @return array<string, mixed> The requested node, containing these fields:
 *
 *   index    the node id, equal to $index
 *   kind     for non-roots, the node's HeaderKind. For roots, "Root".
 *   size     the node's allocated size (including padding), in bytes.
 *   type     if known, the node's C++ type name
 *
 * if the node is an object:
 *   class    PHP classname of the object
 *
 * if the node is a static property (type == HPHP::StaticPropData)
 *   class    PHP classname owning the static property
 *   prop     name of the static property
 *
 */
<<__Native>>
function heapgraph_node(resource $heapgraph, int $index): darray<string, mixed>;

/**
 * Get a specific edge (pointer) from the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param int $index - The edge index
 *
 * @return array<string, mixed> The requested edge, with these fields:
 *
 *   index    the edge id == $index
 *   kind     Counted, Implicit, or Ambiguous
 *   from     node id owning the pointer
 *   to       node id of the node pointed to
 *
 * If the from node is an array:
 *   key      num; this pointer is the num'th key in iterator order (0..)
 *   value    num; pointer is the num'th value in iter order
 *
 * if the from node is an object:
 *   prop     declared property name of the pointer
 *
 * optionally, the pointer may be unclassified, but with a known offset:
 *   offset   byte-offset of the pointer within the from node.
 *
 */
<<__Native>>
function heapgraph_edge(resource $heapgraph, int $index): darray<string, mixed>;

/**
 * Get an array of incoming edges to a specific node from the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param int $index - The node index
 *
 * @return array<array<string, mixed>> The incoming edges
 * See documentation for heapgraph_edge() about the "edge" array passed
 * to $callback
 */
<<__Native>>
function heapgraph_node_in_edges(
  resource $heapgraph,
  int $index
): varray<darray<string, mixed>>;

/**
 * Get an array of outgoing edges from a specific node from the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param int $index - The node index
 *
 * @return array<array<string, mixed>> The outgoing edges
 * See documentation for heapgraph_edge() about the "edge" array passed
 * to $callback
 */
<<__Native>>
function heapgraph_node_out_edges(
  resource $heapgraph,
  int $index
): varray<darray<string, mixed>>;

}
