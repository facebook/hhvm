
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get an array of outgoing edges from a specific node from the heap graph







``` Hack
namespace HH;

function heapgraph_node_out_edges(
  resource $heapgraph,
  int $index,
): varray<darray<string, mixed>>;
```




## Parameters




+ ` resource $heapgraph ` - The resource obtained with heapgraph_create
+ ` int $index ` - The node index




## Returns




* ` array<array<string, ` - mixed>> The outgoing edges
  See documentation for heapgraph_edge() about the "edge" array passed
  to $callback
<!-- HHAPIDOC -->
