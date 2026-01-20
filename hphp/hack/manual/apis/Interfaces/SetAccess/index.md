---
title: SetAccess
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The interface for mutable [` Set `](/apis/Classes/HH/Set/)s to enable removal of its values




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface SetAccess implements ConstSetAccess<Tm> {...}
```




### Public Methods




* [` ->remove(Tm $m): this `](/apis/Interfaces/SetAccess/remove/)\
  Removes the provided value from the current [` Set `](/apis/Classes/HH/Set/)







### Public Methods ([` ConstSetAccess `](/apis/Interfaces/ConstSetAccess/))




- [` ->contains(arraykey $m): bool `](/apis/Interfaces/ConstSetAccess/contains/)\
  Checks whether a value is in the current [` Set `](/apis/Classes/HH/Set/)
<!-- HHAPIDOC -->
