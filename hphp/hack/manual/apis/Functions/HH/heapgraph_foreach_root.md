
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Iterates over the root edges in the heap graph




``` Hack
namespace HH;

function heapgraph_foreach_root(
  resource $heapgraph,
  mixed $callback,
): void;
```




Calls back on every root
edge. The "from" field of each root edge will be root node. root nodes
are also enumerable by calling heapgraph_foreach_root_node().




## Parameters




+ ` resource $heapgraph ` - The resource obtained with heapgraph_create
+ ` mixed $callback ` - `function(darray<string, mixed> $edge): void {}`
  See documentation for heapgraph_edge() about the "edge" array passed
  to $callback




## Returns




* ` void `
<!-- HHAPIDOC -->
