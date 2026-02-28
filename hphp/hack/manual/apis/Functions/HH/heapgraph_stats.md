
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

General statistics on the heap graph







``` Hack
namespace HH;

function heapgraph_stats(
  resource $heapgraph,
): darray<string, int>;
```




## Parameters




+ ` resource $heapgraph ` - The resource obtained with heapgraph_create




## Returns




* ` array<string, ` - int> - General metrics describing the heap graph.
  Keys are defined as follows:




nodes      count of nodes in the graph
edges      count of pointers (edges) in the graph
roots      count of root->nonroot pointers (pointers where from is root)
root_nodes count of root nodes
exact      1 if type scanners were built, 0 otherwise
<!-- HHAPIDOC -->
