
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get a specific edge (pointer) from the heap graph







``` Hack
namespace HH;

function heapgraph_edge(
  resource $heapgraph,
  int $index,
): darray<string, mixed>;
```




## Parameters




+ ` resource $heapgraph ` - The resource obtained with heapgraph_create
+ ` int $index ` - The edge index




## Returns




* ` array<string, ` - mixed> The requested edge, with these fields:




index    the edge id == $index
kind     Counted, Implicit, or Ambiguous
from     node id owning the pointer
to       node id of the node pointed to




If the from node is an array:
key      num; this pointer is the num'th key in iterator order (0..)
value    num; pointer is the num'th value in iter order




if the from node is an object:
prop     declared property name of the pointer




optionally, the pointer may be unclassified, but with a known offset:
offset   byte-offset of the pointer within the from node.
<!-- HHAPIDOC -->
