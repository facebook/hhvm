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




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface ConstMap implements ConstCollection<Pair<Tk, Tv>>, ConstMapAccess<Tk, Tv>, HH\KeyedIterable<Tk, Tv>, HH\KeyedContainer<Tk, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): ConstVector<Tu> `](/apis/Interfaces/ConstMap/concat/)\
  Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) that is the concatenation of the values of the
  current `` ConstMap `` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): ConstMap<Tk, Tv> `](/apis/Interfaces/ConstMap/filter/)\
  Returns a `` ConstMap `` containing the values of the current ``` ConstMap ``` that
  meet a supplied condition
* [` ->filterWithKey((function(Tk, Tv): bool) $fn): ConstMap<Tk, Tv> `](/apis/Interfaces/ConstMap/filterWithKey/)\
  Returns a `` ConstMap `` containing the values of the current ``` ConstMap ``` that
  meet a supplied condition applied to its keys and values
* [` ->firstKey(): ?Tk `](/apis/Interfaces/ConstMap/firstKey/)\
  Returns the first key in the current `` ConstMap ``
* [` ->firstValue(): ?Tv `](/apis/Interfaces/ConstMap/firstValue/)\
  Returns the first value in the current `` ConstMap ``
* [` ->keys(): ConstVector<Tk> `](/apis/Interfaces/ConstMap/keys/)\
  Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the keys of the current `` ConstMap ``
* [` ->lastKey(): ?Tk `](/apis/Interfaces/ConstMap/lastKey/)\
  Returns the last key in the current `` ConstMap ``
* [` ->lastValue(): ?Tv `](/apis/Interfaces/ConstMap/lastValue/)\
  Returns the last value in the current `` ConstMap ``
* [` ->map<Tu>((function(Tv): Tu) $fn): ConstMap<Tk, Tu> `](/apis/Interfaces/ConstMap/map/)\
  Returns a `` ConstMap `` after an operation has been applied to each value in
  the current ``` ConstMap ```
* [` ->mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): ConstMap<Tk, Tu> `](/apis/Interfaces/ConstMap/mapWithKey/)\
  Returns a `` ConstMap `` after an operation has been applied to each key and
  value in the current ``` ConstMap ```
* [` ->skip(int $n): ConstMap<Tk, Tv> `](/apis/Interfaces/ConstMap/skip/)\
  Returns a `` ConstMap `` containing the values after the ``` n ```-th element of the
  current ```` ConstMap ````
* [` ->skipWhile((function(Tv): bool) $fn): ConstMap<Tk, Tv> `](/apis/Interfaces/ConstMap/skipWhile/)\
  Returns a `` ConstMap `` containing the values of the current ``` ConstMap ```
  starting after and including the first value that produces ```` true ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): ConstMap<Tk, Tv> `](/apis/Interfaces/ConstMap/slice/)\
  Returns a subset of the current `` ConstMap `` starting from a given key
  location up to, but not including, the element at the provided length from
  the starting key location
* [` ->take(int $n): ConstMap<Tk, Tv> `](/apis/Interfaces/ConstMap/take/)\
  Returns a `` ConstMap `` containing the first ``` n ``` key/values of the current
  ```` ConstMap ````
* [` ->takeWhile((function(Tv): bool) $fn): ConstMap<Tk, Tv> `](/apis/Interfaces/ConstMap/takeWhile/)\
  Returns a `` ConstMap `` containing the keys and values of the current
  ``` ConstMap ``` up to but not including the first value that produces ```` false ````
  when passed to the specified callback
* [` ->toDArray(): darray<Tk, Tv> `](/apis/Interfaces/ConstMap/toDArray/)
* [` ->toVArray(): varray<Tv> `](/apis/Interfaces/ConstMap/toVArray/)
* [` ->values(): ConstVector<Tv> `](/apis/Interfaces/ConstMap/values/)\
  Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the values of the current `` ConstMap ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): ConstMap<Tk, Pair<Tv, Tu>> `](/apis/Interfaces/ConstMap/zip/)\
  Returns a `` ConstMap `` where each value is a [` Pair `](/apis/Classes/HH/Pair/) that combines the value
  of the current `` ConstMap `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)







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
<!-- HHAPIDOC -->
