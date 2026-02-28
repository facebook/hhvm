
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get a specific node from the heap graph







``` Hack
namespace HH;

function heapgraph_node(
  resource $heapgraph,
  int $index,
): darray<string, mixed>;
```




## Parameters




+ ` resource $heapgraph ` - The resource obtained with heapgraph_create
+ ` int $index ` - The node index




## Returns




* ` array<string, ` - mixed> The requested node, containing these fields:




index    the node id, equal to $index
kind     for non-roots, the node's HeaderKind. For roots, "Root".
size     the node's allocated size (including padding), in bytes.
type     if known, the node's C++ type name




if the node is an object:
class    PHP classname of the object




if the node is a static property (type == HPHP::StaticPropData)
class    PHP classname owning the static property
prop     name of the static property
<!-- HHAPIDOC -->
