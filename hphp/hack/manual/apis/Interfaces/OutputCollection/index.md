---
title: OutputCollection
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The interface implemented for a mutable collection type so that values can
be added to it




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface OutputCollection {...}
```




### Public Methods




* [` ->add(Te $e): this `](/apis/Interfaces/OutputCollection/add/)\
  Add a value to the collection and return the collection itself
* [` ->addAll(?Traversable<Te> $traversable): this `](/apis/Interfaces/OutputCollection/addAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), append a value into the
  current collection
<!-- HHAPIDOC -->
