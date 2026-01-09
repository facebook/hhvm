---
title: DOMNamedNodeMap
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
class DOMNamedNodeMap implements IteratorAggregate<?DOMNode>, KeyedTraversable<string, Tnode> {...}
```




### Public Methods




+ [` ->__construct(): void `](/docs/apis/Classes/DOMNamedNodeMap/__construct/)
+ [` ->getIterator(): DOMNodeIterator `](/docs/apis/Classes/DOMNamedNodeMap/getIterator/)
+ [` ->getNamedItem(string $name): Tnode `](/docs/apis/Classes/DOMNamedNodeMap/getNamedItem/)\
  Retrieves a node specified by its nodeName
+ [` ->getNamedItemNS(string $namespaceuri, string $localname): Tnode `](/docs/apis/Classes/DOMNamedNodeMap/getNamedItemNS/)\
  Retrieves a node specified by localName and namespaceURI
+ [` ->item(int $index): Tnode `](/docs/apis/Classes/DOMNamedNodeMap/item/)\
  Retrieves a node specified by index within the DOMNamedNodeMap object
<!-- HHAPIDOC -->
