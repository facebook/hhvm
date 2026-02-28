
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This function inserts a new node right before the reference node




``` Hack
public function insertBefore<T as DOMNode>(
  DOMNode $newnode,
  DOMNode $refnode = NULL,
): T;
```




If you
plan to do further modifications on the appended child you must use the
returned node.




## Parameters




+ [` DOMNode `](/apis/Classes/DOMNode/)`` $newnode `` - The new node.
+ [` DOMNode `](/apis/Classes/DOMNode/)`` $refnode = NULL `` - The reference node. If not supplied, newnode is
  appended to the children.




## Returns




* ` mixed ` - - The inserted node.
<!-- HHAPIDOC -->
