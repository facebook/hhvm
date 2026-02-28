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
[` OutputCollection `](/apis/Interfaces/OutputCollection/) only. If your collection to be immutable, implement
[` ConstCollection `](/apis/Interfaces/ConstCollection/) only instead.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface Collection implements \ConstCollection<Te>, \OutputCollection<Te> {...}
```




### Public Methods




* [` ->clear() `](/apis/Interfaces/HH/Collection/clear/)\
  Removes all items from the collection







### Public Methods ([` ConstCollection `](/apis/Interfaces/ConstCollection/))




- [` ->count(): int `](/apis/Interfaces/ConstCollection/count/)\
  Get the number of items in the collection

- [` ->isEmpty(): bool `](/apis/Interfaces/ConstCollection/isEmpty/)\
  Is the collection empty?

- [` ->items(): HH\Iterable<Te> `](/apis/Interfaces/ConstCollection/items/)\
  Get access to the items in the collection

- [` ->toDArray(): darray `](/apis/Interfaces/ConstCollection/toDArray/)

- [` ->toVArray(): varray `](/apis/Interfaces/ConstCollection/toVArray/)








### Public Methods ([` IPureStringishObject `](/apis/Interfaces/IPureStringishObject/))




+ [` ->__toString(): string `](/apis/Interfaces/IPureStringishObject/__toString/)







### Public Methods ([` OutputCollection `](/apis/Interfaces/OutputCollection/))




* [` ->add(Te $e): this `](/apis/Interfaces/OutputCollection/add/)\
  Add a value to the collection and return the collection itself
* [` ->addAll(?Traversable<Te> $traversable): this `](/apis/Interfaces/OutputCollection/addAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), append a value into the
  current collection
<!-- HHAPIDOC -->
