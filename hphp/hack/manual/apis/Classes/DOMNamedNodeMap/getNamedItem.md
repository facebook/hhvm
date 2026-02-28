
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieves a node specified by its nodeName




``` Hack
public function getNamedItem(
  string $name,
): Tnode;
```




## Parameters




+ ` string $name ` - The nodeName of the node to retrieve.




## Returns




* ` mixed ` - - A node (of any type) with the specified nodeName, or NULL
  if no node is found.
<!-- HHAPIDOC -->
