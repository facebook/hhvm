
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieves a node specified by index within the DOMNamedNodeMap object




``` Hack
public function item(
  int $index,
): Tnode;
```




## Parameters




+ ` int $index ` - Index into this map.




## Returns




* ` mixed ` - - The node at the indexth position in the map, or NULL if
  that is not a valid index (greater than or equal to the number of nodes
  in this map).
<!-- HHAPIDOC -->
