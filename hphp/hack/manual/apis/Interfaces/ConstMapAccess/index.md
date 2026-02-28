---
title: ConstMapAccess
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The interface for enabling access to the [` Map `](/apis/Classes/HH/Map/)s values




This interface provides no new methods as all current access for [` Map `](/apis/Classes/HH/Map/)s are
defined in its parent interfaces. But you could theoretically use this
interface for parameter and return type annotations.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface ConstMapAccess implements ConstSetAccess<Tk>, ConstIndexAccess<Tk, Tv> {...}
```




### Public Methods ([` ConstSetAccess `](/apis/Interfaces/ConstSetAccess/))




* [` ->contains(arraykey $m): bool `](/apis/Interfaces/ConstSetAccess/contains/)\
  Checks whether a value is in the current [` Set `](/apis/Classes/HH/Set/)







### Public Methods ([` ConstIndexAccess `](/apis/Interfaces/ConstIndexAccess/))




- [` ->at(Tk $k): Tv `](/apis/Interfaces/ConstIndexAccess/at/)\
  Returns the value at the specified key in the current collection
- [` ->containsKey(mixed $k): bool `](/apis/Interfaces/ConstIndexAccess/containsKey/)\
  Determines if the specified key is in the current collection
- [` ->get(Tk $k): ?Tv `](/apis/Interfaces/ConstIndexAccess/get/)\
  Returns the value at the specified key in the current collection
<!-- HHAPIDOC -->
