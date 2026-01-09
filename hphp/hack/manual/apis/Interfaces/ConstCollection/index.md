---
title: ConstCollection
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The base interface implemented for a collection type so that base information
such as count and its items are available




Every concrete class indirectly
implements this interface.




## Guides




+ [Introduction](</docs/hack/arrays-and-collections/introduction>)
+ [Interfaces](</docs/hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface ConstCollection implements Countable, IPureStringishObject {...}
```




### Public Methods




* [` ->count(): int `](/docs/apis/Interfaces/ConstCollection/count/)\
  Get the number of items in the collection

* [` ->isEmpty(): bool `](/docs/apis/Interfaces/ConstCollection/isEmpty/)\
  Is the collection empty?

* [` ->items(): HH\Iterable<Te> `](/docs/apis/Interfaces/ConstCollection/items/)\
  Get access to the items in the collection

* [` ->toDArray(): darray `](/docs/apis/Interfaces/ConstCollection/toDArray/)

* [` ->toVArray(): varray `](/docs/apis/Interfaces/ConstCollection/toVArray/)








### Public Methods ([` IPureStringishObject `](/docs/apis/Interfaces/IPureStringishObject/))




- [` ->__toString(): string `](/docs/apis/Interfaces/IPureStringishObject/__toString/)
<!-- HHAPIDOC -->
