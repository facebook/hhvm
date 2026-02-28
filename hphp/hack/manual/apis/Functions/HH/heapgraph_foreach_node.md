
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Iterates over the nodes in the heap graph




``` Hack
namespace HH;

function heapgraph_foreach_node(
  resource $heapgraph,
  mixed $callback,
): void;
```




Calls back on every node.




## Parameters




+ ` resource $heapgraph ` - The resource obtained with heapgraph_create
+ ` mixed $callback ` - `function(array<string, mixed> $node): void {}`
  See documentation for heapgraph_node() about the "node" array passed
  to $callback




## Returns




* ` void `
<!-- HHAPIDOC -->
