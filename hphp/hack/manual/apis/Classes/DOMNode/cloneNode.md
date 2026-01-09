
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates a copy of the node




``` Hack
public function cloneNode(
  bool $deep = false,
): this;
```




## Parameters




+ ` bool $deep = false ` - Indicates whether to copy all descendant nodes. This
  parameter is defaulted to FALSE.




## Returns




* ` mixed ` - - The cloned node.
<!-- HHAPIDOC -->
