
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return an integer describing the current format of stats, nodes,
and edges




``` Hack
namespace HH;

function heapgraph_version(): int;
```




This will be incremented when things change substantially
enough to require client detection




## Returns




+ ` int `
<!-- HHAPIDOC -->
