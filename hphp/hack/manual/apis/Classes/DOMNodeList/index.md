---
title: DOMNodeList
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
class DOMNodeList implements IteratorAggregate<?DOMNode, Tnode> {...}
```




### Public Methods




+ [` ->__construct(): void `](/apis/Classes/DOMNodeList/__construct/)
+ [` ->__debugInfo(): darray<string, mixed> `](/apis/Classes/DOMNodeList/__debugInfo/)
+ [` ->getIterator(): Iterator<Tnode> `](/apis/Classes/DOMNodeList/getIterator/)\
  Actually returns DOMNodeIterator which is not exposed as an HHI
+ [` ->item(int $index): Tnode `](/apis/Classes/DOMNodeList/item/)\
  Retrieves a node specified by index within the DOMNodeList object
<!-- HHAPIDOC -->
