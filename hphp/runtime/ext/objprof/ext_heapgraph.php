<?hh

namespace HH {

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
 */
<<__Native>>
function heapgraph_stats(resource $heapgraph): array<string, int>;

/**
 * Iterates over the nodes in the heap graph. Calls back on every node.
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param mixed $callback - function(array<string, mixed> $node): void {}
 */
<<__Native>>
function heapgraph_foreach_node(resource $heapgraph, mixed $callback): void;

/**
 * Iterates over the edges in the heap graph. Calls back on every edge.
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param mixed $callback - function(array<string, mixed> $edge): void {}
 */
<<__Native>>
function heapgraph_foreach_edge(resource $heapgraph, mixed $callback): void;

/**
 * Iterates over the roots in the heap graph. Calls back on every root.
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param mixed $callback - function(array<string, mixed> $edge): void {}
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
 */
<<__Native>>
function heapgraph_dfs_nodes(
  resource $heapgraph,
  array<int> $roots,
  array<int> $skips,
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
 */
<<__Native>>
function heapgraph_dfs_edges(
  resource $heapgraph,
  array<int> $roots,
  array<int> $skips,
  mixed $callback
): void;

/**
 * Get a specific node from the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param int $index - The node index
 *
 * @return array<string, mixed> The requested node
 */
<<__Native>>
function heapgraph_node(resource $heapgraph, int $index): array<string, mixed>;

/**
 * Get a specific edge from the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param int $index - The edge index
 *
 * @return array<string, mixed> The requested edge
 */
<<__Native>>
function heapgraph_edge(resource $heapgraph, int $index): array<string, mixed>;

/**
 * Get an array of incoming edges to a specific node from the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param int $index - The node index
 *
 * @return array<array<string, mixed>> The incoming edges
 */
<<__Native>>
function heapgraph_node_in_edges(
  resource $heapgraph,
  int $index
): array<array<string, mixed>>;

/**
 * Get an array of outgoing edges from a specific node from the heap graph
 *
 * @param resource $heapgraph - The resource obtained with heapgraph_create
 * @param int $index - The node index
 *
 * @return array<array<string, mixed>> The outgoing edges
 */
<<__Native>>
function heapgraph_node_out_edges(
  resource $heapgraph,
  int $index
): array<array<string, mixed>>;

}
