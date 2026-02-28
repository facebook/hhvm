
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This functions appends a child to an existing list of children or creates
a new list of children




``` Hack
public function appendChild<T as DOMNode>(
  DOMNode $newnode,
): T;
```




The child can be created with e.g.
DOMDocument::createElement(), DOMDocument::createTextNode() etc. or
simply by using any other node.




## Parameters




+ [` DOMNode `](/apis/Classes/DOMNode/)`` $newnode `` - The appended child.




## Returns




* ` mixed ` - - The node added.
<!-- HHAPIDOC -->
