---
title: MutableMap
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a write-enabled (mutable) sequence of key/value pairs
(i.e. a map)




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface MutableMap implements ConstMap<Tk, Tv>, HH\Collection<Pair<Tk, Tv>>, MapAccess<Tk, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): MutableVector<Tu> `](/apis/Interfaces/MutableMap/concat/)\
  Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) that is the concatenation of the values of the
  current `` MutableMap `` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): MutableMap<Tk, Tv> `](/apis/Interfaces/MutableMap/filter/)\
  Returns a `` MutableMap `` containing the values of the current ``` MutableMap ```
  that meet a supplied condition
* [` ->filterWithKey((function(Tk, Tv): bool) $fn): MutableMap<Tk, Tv> `](/apis/Interfaces/MutableMap/filterWithKey/)\
  Returns a `` MutableMap `` containing the values of the current ``` MutableMap ```
  that meet a supplied condition applied to its keys and values
* [` ->firstKey(): ?Tk `](/apis/Interfaces/MutableMap/firstKey/)\
  Returns the first key in the current `` MutableMap ``
* [` ->firstValue(): ?Tv `](/apis/Interfaces/MutableMap/firstValue/)\
  Returns the first value in the current `` MutableMap ``
* [` ->keys(): MutableVector<Tk> `](/apis/Interfaces/MutableMap/keys/)\
  Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the keys of the current `` MutableMap ``
* [` ->lastKey(): ?Tk `](/apis/Interfaces/MutableMap/lastKey/)\
  Returns the last key in the current `` MutableMap ``
* [` ->lastValue(): ?Tv `](/apis/Interfaces/MutableMap/lastValue/)\
  Returns the last value in the current `` MutableMap ``
* [` ->map<Tu>((function(Tv): Tu) $fn): MutableMap<Tk, Tu> `](/apis/Interfaces/MutableMap/map/)\
  Returns a `` MutableMap `` after an operation has been applied to each value
  in the current ``` MutableMap ```
* [` ->mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): MutableMap<Tk, Tu> `](/apis/Interfaces/MutableMap/mapWithKey/)\
  Returns a `` MutableMap `` after an operation has been applied to each key and
  value in the current ``` MutableMap ```
* [` ->skip(int $n): MutableMap<Tk, Tv> `](/apis/Interfaces/MutableMap/skip/)\
  Returns a `` MutableMap `` containing the values after the ``` n ```-th element of
  the current ```` MutableMap ````
* [` ->skipWhile((function(Tv): bool) $fn): MutableMap<Tk, Tv> `](/apis/Interfaces/MutableMap/skipWhile/)\
  Returns a `` MutableMap `` containing the values of the current ``` MutableMap ```
  starting after and including the first value that produces ```` true ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): MutableMap<Tk, Tv> `](/apis/Interfaces/MutableMap/slice/)\
  Returns a subset of the current `` MutableMap `` starting from a given key
  location up to, but not including, the element at the provided length from
  the starting key location
* [` ->take(int $n): MutableMap<Tk, Tv> `](/apis/Interfaces/MutableMap/take/)\
  Returns a `` MutableMap `` containing the first ``` n ``` key/values of the current
  ```` MutableMap ````
* [` ->takeWhile((function(Tv): bool) $fn): MutableMap<Tk, Tv> `](/apis/Interfaces/MutableMap/takeWhile/)\
  Returns a `` MutableMap `` containing the keys and values of the current
  ``` MutableMap ``` up to but not including the first value that produces ```` false ````
  when passed to the specified callback
* [` ->toDArray(): darray<Tk, Tv> `](/apis/Interfaces/MutableMap/toDArray/)
* [` ->toVArray(): varray<Tv> `](/apis/Interfaces/MutableMap/toVArray/)
* [` ->values(): MutableVector<Tv> `](/apis/Interfaces/MutableMap/values/)\
  Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values of the current
  `` MutableMap ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): MutableMap<Tk, Pair<Tv, Tu>> `](/apis/Interfaces/MutableMap/zip/)\
  Returns a `` MutableMap `` where each value is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
  value of the current `` MutableMap `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)







### Public Methods ([` ConstCollection `](/apis/Interfaces/ConstCollection/))




- [` ->count(): int `](/apis/Interfaces/ConstCollection/count/)\
  Get the number of items in the collection

- [` ->isEmpty(): bool `](/apis/Interfaces/ConstCollection/isEmpty/)\
  Is the collection empty?

- [` ->items(): HH\Iterable<Te> `](/apis/Interfaces/ConstCollection/items/)\
  Get access to the items in the collection








### Public Methods ([` IPureStringishObject `](/apis/Interfaces/IPureStringishObject/))




+ [` ->__toString(): string `](/apis/Interfaces/IPureStringishObject/__toString/)







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







### Public Methods ([` HH\KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/))




+ [` ->getIterator(): KeyedIterator<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/getIterator/)\
  Returns an iterator that points to beginning of the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)
+ [` ->lazy(): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)
+ [` ->toImmMap(): ImmMap<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/toImmMap/)\
  Returns an immutable map ([` ImmMap `](/apis/Classes/HH/ImmMap/)) based on the keys and values of the
  current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)
+ [` ->toKeysArray(): varray `](/apis/Interfaces/HH/KeyedIterable/toKeysArray/)\
  Returns an `` array `` with the keys from the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)







### Public Methods ([` HH\Iterable `](/apis/Interfaces/HH/Iterable/))




* [` ->toImmSet(): ImmSet<Tv> `](/apis/Interfaces/HH/Iterable/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/apis/Classes/HH/ImmSet/)) converted from the current [` Iterable `](/apis/Interfaces/HH/Iterable/)
* [` ->toImmVector(): ImmVector<Tv> `](/apis/Interfaces/HH/Iterable/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) converted from the current
  [` Iterable `](/apis/Interfaces/HH/Iterable/)
* [` ->toValuesArray(): varray<Tv> `](/apis/Interfaces/HH/Iterable/toValuesArray/)\
  Returns an `` array `` with the values from the current [` Iterable `](/apis/Interfaces/HH/Iterable/)







### Public Methods ([` HH\Collection `](/apis/Interfaces/HH/Collection/))




- [` ->clear() `](/apis/Interfaces/HH/Collection/clear/)\
  Removes all items from the collection







### Public Methods ([` OutputCollection `](/apis/Interfaces/OutputCollection/))




+ [` ->add(Te $e): this `](/apis/Interfaces/OutputCollection/add/)\
  Add a value to the collection and return the collection itself
+ [` ->addAll(?Traversable<Te> $traversable): this `](/apis/Interfaces/OutputCollection/addAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), append a value into the
  current collection







### Public Methods ([` SetAccess `](/apis/Interfaces/SetAccess/))




* [` ->remove(Tm $m): this `](/apis/Interfaces/SetAccess/remove/)\
  Removes the provided value from the current [` Set `](/apis/Classes/HH/Set/)







### Public Methods ([` IndexAccess `](/apis/Interfaces/IndexAccess/))




- [` ->removeKey(Tk $k): this `](/apis/Interfaces/IndexAccess/removeKey/)\
  Removes the specified key (and associated value) from the current
  collection
- [` ->set(Tk $k, Tv $v): this `](/apis/Interfaces/IndexAccess/set/)\
  Stores a value into the current collection with the specified key,
  overwriting the previous value associated with the key
- [` ->setAll(?KeyedTraversable<Tk, Tv> $traversable): this `](/apis/Interfaces/IndexAccess/setAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), stores a value into the
  current collection associated with each key, overwriting the previous value
  associated with the key
<!-- HHAPIDOC -->
