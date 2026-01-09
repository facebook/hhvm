
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

performs dfs over the heap graph, starting from the given nodes




``` Hack
namespace HH;

function heapgraph_dfs_nodes(
  resource $heapgraph,
  array<int> $roots,
  array<int> $skips,
  mixed $callback,
): void;
```




calls back on every new node in the scan.




## Parameters




+ ` resource $heapgraph ` - the resource obtained with heapgraph_create
+ ` array<int> $roots ` - node indexes to start the scan from
+ ` array<int> $skips ` - node indexes to consider as if they're not there
+ ` mixed $callback ` - `function(array<string, mixed> $node): void {}`
  See documentation for heapgraph_edge() about the "edge" array passed
  to $callback




## Returns




* ` void `
<!-- HHAPIDOC -->
