
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This function replaces the child oldnode with the passed new node




``` Hack
public function replaceChild<T as DOMNode>(
  DOMNode $newchildobj,
  DOMNode $oldchildobj,
): T;
```




If the
new node is already a child it will not be added a second time. If the
replacement succeeds the old node is returned.




## Parameters




+ [` DOMNode `](/apis/Classes/DOMNode/)`` $newchildobj `` - The new node. It must be a member of the
  target document, i.e. created by one of the DOMDocument->createXXX()
  methods or imported in the document by DOMDocument::importNode.
+ [` DOMNode `](/apis/Classes/DOMNode/)`` $oldchildobj `` - The old node.




## Returns




* ` mixed ` - - The old node or FALSE if an error occur.
<!-- HHAPIDOC -->
