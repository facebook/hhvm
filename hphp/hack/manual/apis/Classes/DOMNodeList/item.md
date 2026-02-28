
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieves a node specified by index within the DOMNodeList object




``` Hack
public function item(
  int $index,
): Tnode;
```




Tip
If you need to know the number of nodes in the collection, use the length
property of the DOMNodeList object.




## Parameters




+ ` int $index ` - Index of the node into the collection.




## Returns




* ` mixed ` - - The node at the indexth position in the DOMNodeList, or
  NULL if that is not a valid index.
<!-- HHAPIDOC -->
