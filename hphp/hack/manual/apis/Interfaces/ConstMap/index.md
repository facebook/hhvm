---
title: ConstMap
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a read-only (immutable) sequence of key/value pairs, (i.e. a map)




## Guides




+ [Introduction](</docs/hack/arrays-and-collections/introduction>)
+ [Interfaces](</docs/hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface ConstMap implements ConstCollection<Pair<Tk, Tv>>, ConstMapAccess<Tk, Tv>, HH\KeyedIterable<Tk, Tv>, HH\KeyedContainer<Tk, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): ConstVector<Tu> `](/docs/apis/Interfaces/ConstMap/concat/)\
  Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) that is the concatenation of the values of the
  current `` ConstMap `` and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): ConstMap<Tk, Tv> `](/docs/apis/Interfaces/ConstMap/filter/)\
  Returns a `` ConstMap `` containing the values of the current ``` ConstMap ``` that
  meet a supplied condition
* [` ->filterWithKey((function(Tk, Tv): bool) $fn): ConstMap<Tk, Tv> `](/docs/apis/Interfaces/ConstMap/filterWithKey/)\
  Returns a `` ConstMap `` containing the values of the current ``` ConstMap ``` that
  meet a supplied condition applied to its keys and values
* [` ->firstKey(): ?Tk `](/docs/apis/Interfaces/ConstMap/firstKey/)\
  Returns the first key in the current `` ConstMap ``
* [` ->firstValue(): ?Tv `](/docs/apis/Interfaces/ConstMap/firstValue/)\
  Returns the first value in the current `` ConstMap ``
* [` ->keys(): ConstVector<Tk> `](/docs/apis/Interfaces/ConstMap/keys/)\
  Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the keys of the current `` ConstMap ``
* [` ->lastKey(): ?Tk `](/docs/apis/Interfaces/ConstMap/lastKey/)\
  Returns the last key in the current `` ConstMap ``
* [` ->lastValue(): ?Tv `](/docs/apis/Interfaces/ConstMap/lastValue/)\
  Returns the last value in the current `` ConstMap ``
* [` ->map<Tu>((function(Tv): Tu) $fn): ConstMap<Tk, Tu> `](/docs/apis/Interfaces/ConstMap/map/)\
  Returns a `` ConstMap `` after an operation has been applied to each value in
  the current ``` ConstMap ```
* [` ->mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): ConstMap<Tk, Tu> `](/docs/apis/Interfaces/ConstMap/mapWithKey/)\
  Returns a `` ConstMap `` after an operation has been applied to each key and
  value in the current ``` ConstMap ```
* [` ->skip(int $n): ConstMap<Tk, Tv> `](/docs/apis/Interfaces/ConstMap/skip/)\
  Returns a `` ConstMap `` containing the values after the ``` n ```-th element of the
  current ```` ConstMap ````
* [` ->skipWhile((function(Tv): bool) $fn): ConstMap<Tk, Tv> `](/docs/apis/Interfaces/ConstMap/skipWhile/)\
  Returns a `` ConstMap `` containing the values of the current ``` ConstMap ```
  starting after and including the first value that produces ```` true ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): ConstMap<Tk, Tv> `](/docs/apis/Interfaces/ConstMap/slice/)\
  Returns a subset of the current `` ConstMap `` starting from a given key
  location up to, but not including, the element at the provided length from
  the starting key location
* [` ->take(int $n): ConstMap<Tk, Tv> `](/docs/apis/Interfaces/ConstMap/take/)\
  Returns a `` ConstMap `` containing the first ``` n ``` key/values of the current
  ```` ConstMap ````
* [` ->takeWhile((function(Tv): bool) $fn): ConstMap<Tk, Tv> `](/docs/apis/Interfaces/ConstMap/takeWhile/)\
  Returns a `` ConstMap `` containing the keys and values of the current
  ``` ConstMap ``` up to but not including the first value that produces ```` false ````
  when passed to the specified callback
* [` ->toDArray(): darray<Tk, Tv> `](/docs/apis/Interfaces/ConstMap/toDArray/)
* [` ->toVArray(): varray<Tv> `](/docs/apis/Interfaces/ConstMap/toVArray/)
* [` ->values(): ConstVector<Tv> `](/docs/apis/Interfaces/ConstMap/values/)\
  Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the values of the current `` ConstMap ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): ConstMap<Tk, Pair<Tv, Tu>> `](/docs/apis/Interfaces/ConstMap/zip/)\
  Returns a `` ConstMap `` where each value is a [` Pair `](/docs/apis/Classes/HH/Pair/) that combines the value
  of the current `` ConstMap `` and the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)







### Public Methods ([` ConstCollection `](/docs/apis/Interfaces/ConstCollection/))




- [` ->count(): int `](/docs/apis/Interfaces/ConstCollection/count/)\
  Get the number of items in the collection

- [` ->isEmpty(): bool `](/docs/apis/Interfaces/ConstCollection/isEmpty/)\
  Is the collection empty?

- [` ->items(): HH\Iterable<Te> `](/docs/apis/Interfaces/ConstCollection/items/)\
  Get access to the items in the collection








### Public Methods ([` IPureStringishObject `](/docs/apis/Interfaces/IPureStringishObject/))




+ [` ->__toString(): string `](/docs/apis/Interfaces/IPureStringishObject/__toString/)







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







### Public Methods ([` HH\KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/))




+ [` ->getIterator(): KeyedIterator<Tk, Tv> `](/docs/apis/Interfaces/HH/KeyedIterable/getIterator/)\
  Returns an iterator that points to beginning of the current
  [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/)
+ [` ->lazy(): KeyedIterable<Tk, Tv> `](/docs/apis/Interfaces/HH/KeyedIterable/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/)
+ [` ->toImmMap(): ImmMap<Tk, Tv> `](/docs/apis/Interfaces/HH/KeyedIterable/toImmMap/)\
  Returns an immutable map ([` ImmMap `](/docs/apis/Classes/HH/ImmMap/)) based on the keys and values of the
  current [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/)
+ [` ->toKeysArray(): varray `](/docs/apis/Interfaces/HH/KeyedIterable/toKeysArray/)\
  Returns an `` array `` with the keys from the current [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/)







### Public Methods ([` HH\Iterable `](/docs/apis/Interfaces/HH/Iterable/))




* [` ->toImmSet(): ImmSet<Tv> `](/docs/apis/Interfaces/HH/Iterable/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/docs/apis/Classes/HH/ImmSet/)) converted from the current [` Iterable `](/docs/apis/Interfaces/HH/Iterable/)
* [` ->toImmVector(): ImmVector<Tv> `](/docs/apis/Interfaces/HH/Iterable/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/docs/apis/Classes/HH/ImmVector/)) converted from the current
  [` Iterable `](/docs/apis/Interfaces/HH/Iterable/)
* [` ->toValuesArray(): varray<Tv> `](/docs/apis/Interfaces/HH/Iterable/toValuesArray/)\
  Returns an `` array `` with the values from the current [` Iterable `](/docs/apis/Interfaces/HH/Iterable/)
<!-- HHAPIDOC -->
