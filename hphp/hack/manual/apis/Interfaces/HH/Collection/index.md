---
title: Collection
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` Collection ` is the primary collection interface for mutable collections




Assuming you want the ability to clear out your collection, you would
implement this (or a child of this) interface. Otherwise, you can implement
[` OutputCollection `](/docs/apis/Interfaces/OutputCollection/) only. If your collection to be immutable, implement
[` ConstCollection `](/docs/apis/Interfaces/ConstCollection/) only instead.




## Guides




+ [Introduction](</docs/hack/arrays-and-collections/introduction>)
+ [Interfaces](</docs/hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface Collection implements \ConstCollection<Te>, \OutputCollection<Te> {...}
```




### Public Methods




* [` ->clear() `](/docs/apis/Interfaces/HH/Collection/clear/)\
  Removes all items from the collection







### Public Methods ([` ConstCollection `](/docs/apis/Interfaces/ConstCollection/))




- [` ->count(): int `](/docs/apis/Interfaces/ConstCollection/count/)\
  Get the number of items in the collection

- [` ->isEmpty(): bool `](/docs/apis/Interfaces/ConstCollection/isEmpty/)\
  Is the collection empty?

- [` ->items(): HH\Iterable<Te> `](/docs/apis/Interfaces/ConstCollection/items/)\
  Get access to the items in the collection

- [` ->toDArray(): darray `](/docs/apis/Interfaces/ConstCollection/toDArray/)

- [` ->toVArray(): varray `](/docs/apis/Interfaces/ConstCollection/toVArray/)








### Public Methods ([` IPureStringishObject `](/docs/apis/Interfaces/IPureStringishObject/))




+ [` ->__toString(): string `](/docs/apis/Interfaces/IPureStringishObject/__toString/)







### Public Methods ([` OutputCollection `](/docs/apis/Interfaces/OutputCollection/))




* [` ->add(Te $e): this `](/docs/apis/Interfaces/OutputCollection/add/)\
  Add a value to the collection and return the collection itself
* [` ->addAll(?Traversable<Te> $traversable): this `](/docs/apis/Interfaces/OutputCollection/addAll/)\
  For every element in the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), append a value into the
  current collection
<!-- HHAPIDOC -->
