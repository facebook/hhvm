---
title: MapAccess
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The interface for setting and removing [` Map `](/docs/apis/Classes/HH/Map/) keys (and associated values)




This interface provides no new methods as all current access for [` Map `](/docs/apis/Classes/HH/Map/)s are
defined in its parent interfaces. But you could theoretically use this
interface for parameter and return type annotations.




## Guides




+ [Introduction](</docs/hack/arrays-and-collections/introduction>)
+ [Interfaces](</docs/hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface MapAccess implements ConstMapAccess<Tk, Tv>, SetAccess<Tk>, IndexAccess<Tk, Tv> {...}
```




### Public Methods ([` ConstSetAccess `](/docs/apis/Interfaces/ConstSetAccess/))




* [` ->contains(arraykey $m): bool `](/docs/apis/Interfaces/ConstSetAccess/contains/)\
  Checks whether a value is in the current [` Set `](/docs/apis/Classes/HH/Set/)







### Public Methods ([` ConstIndexAccess `](/docs/apis/Interfaces/ConstIndexAccess/))




- [` ->at(Tk $k): Tv `](/docs/apis/Interfaces/ConstIndexAccess/at/)\
  Returns the value at the specified key in the current collection
- [` ->containsKey(mixed $k): bool `](/docs/apis/Interfaces/ConstIndexAccess/containsKey/)\
  Determines if the specified key is in the current collection
- [` ->get(Tk $k): ?Tv `](/docs/apis/Interfaces/ConstIndexAccess/get/)\
  Returns the value at the specified key in the current collection







### Public Methods ([` SetAccess `](/docs/apis/Interfaces/SetAccess/))




+ [` ->remove(Tm $m): this `](/docs/apis/Interfaces/SetAccess/remove/)\
  Removes the provided value from the current [` Set `](/docs/apis/Classes/HH/Set/)







### Public Methods ([` IndexAccess `](/docs/apis/Interfaces/IndexAccess/))




* [` ->removeKey(Tk $k): this `](/docs/apis/Interfaces/IndexAccess/removeKey/)\
  Removes the specified key (and associated value) from the current
  collection
* [` ->set(Tk $k, Tv $v): this `](/docs/apis/Interfaces/IndexAccess/set/)\
  Stores a value into the current collection with the specified key,
  overwriting the previous value associated with the key
* [` ->setAll(?KeyedTraversable<Tk, Tv> $traversable): this `](/docs/apis/Interfaces/IndexAccess/setAll/)\
  For every element in the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), stores a value into the
  current collection associated with each key, overwriting the previous value
  associated with the key
<!-- HHAPIDOC -->
