---
title: ConstVector
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a read-only (immutable) sequence of values, indexed by integers
(i.e., a vector)




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface ConstVector implements ConstCollection<Tv>, ConstIndexAccess<int, Tv>, HH\KeyedIterable<int, Tv>, HH\KeyedContainer<int, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): ConstVector<Tu> `](/apis/Interfaces/ConstVector/concat/)\
  Returns a `` ConstVector `` that is the concatenation of the values of the
  current ``` ConstVector ``` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): ConstVector<Tv> `](/apis/Interfaces/ConstVector/filter/)\
  Returns a `` ConstVector `` containing the values of the current ``` ConstVector ```
  that meet a supplied condition
* [` ->filterWithKey((function(int, Tv): bool) $fn): ConstVector<Tv> `](/apis/Interfaces/ConstVector/filterWithKey/)\
  Returns a `` ConstVector `` containing the values of the current ``` ConstVector ```
  that meet a supplied condition applied to its keys and values
* [` ->firstKey(): ?int `](/apis/Interfaces/ConstVector/firstKey/)\
  Returns the first key in the current `` ConstVector ``
* [` ->firstValue(): ?Tv `](/apis/Interfaces/ConstVector/firstValue/)\
  Returns the first value in the current `` ConstVector ``
* [` ->keys(): ConstVector<int> `](/apis/Interfaces/ConstVector/keys/)\
  Returns a `` ConstVector `` containing the keys of the current ``` ConstVector ```
* [` ->lastKey(): ?int `](/apis/Interfaces/ConstVector/lastKey/)\
  Returns the last key in the current `` ConstVector ``
* [` ->lastValue(): ?Tv `](/apis/Interfaces/ConstVector/lastValue/)\
  Returns the last value in the current `` ConstVector ``
* [` ->linearSearch(mixed $search_value): int `](/apis/Interfaces/ConstVector/linearSearch/)\
  Returns the index of the first element that matches the search value
* [` ->map<Tu>((function(Tv): Tu) $fn): ConstVector<Tu> `](/apis/Interfaces/ConstVector/map/)\
  Returns a `` ConstVector `` containing the values after an operation has been
  applied to each value in the current ``` ConstVector ```
* [` ->mapWithKey<Tu>((function(int, Tv): Tu) $fn): ConstVector<Tu> `](/apis/Interfaces/ConstVector/mapWithKey/)\
  Returns a `` ConstVector `` containing the values after an operation has been
  applied to each key and value in the current ``` ConstVector ```
* [` ->skip(int $n): ConstVector<Tv> `](/apis/Interfaces/ConstVector/skip/)\
  Returns a `` ConstVector `` containing the values after the ``` n ```-th element of
  the current ```` ConstVector ````
* [` ->skipWhile((function(Tv): bool) $fn): ConstVector<Tv> `](/apis/Interfaces/ConstVector/skipWhile/)\
  Returns a `` ConstVector `` containing the values of the current ``` ConstVector ```
  starting after and including the first value that produces ```` true ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): ConstVector<Tv> `](/apis/Interfaces/ConstVector/slice/)\
  Returns a subset of the current `` ConstVector `` starting from a given key up
  to, but not including, the element at the provided length from the starting
  key
* [` ->take(int $n): ConstVector<Tv> `](/apis/Interfaces/ConstVector/take/)\
  Returns a `` ConstVector `` containing the first ``` n ``` values of the current
  ```` ConstVector ````
* [` ->takeWhile((function(Tv): bool) $fn): ConstVector<Tv> `](/apis/Interfaces/ConstVector/takeWhile/)\
  Returns a `` ConstVector `` containing the values of the current ``` ConstVector ```
  up to but not including the first value that produces ```` false ```` when passed
  to the specified callback
* [` ->toDArray(): darray<int, Tv> `](/apis/Interfaces/ConstVector/toDArray/)
* [` ->toVArray(): varray<Tv> `](/apis/Interfaces/ConstVector/toVArray/)
* [` ->values(): ConstVector<Tv> `](/apis/Interfaces/ConstVector/values/)\
  Returns a `` ConstVector `` containing the values of the current
  ``` ConstVector ```
* [` ->zip<Tu>(Traversable<Tu> $traversable): ConstVector<Pair<Tv, Tu>> `](/apis/Interfaces/ConstVector/zip/)\
  Returns a `` ConstVector `` where each element is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
  element of the current `` ConstVector `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)







### Public Methods ([` ConstCollection `](/apis/Interfaces/ConstCollection/))




- [` ->count(): int `](/apis/Interfaces/ConstCollection/count/)\
  Get the number of items in the collection

- [` ->isEmpty(): bool `](/apis/Interfaces/ConstCollection/isEmpty/)\
  Is the collection empty?

- [` ->items(): HH\Iterable<Te> `](/apis/Interfaces/ConstCollection/items/)\
  Get access to the items in the collection








### Public Methods ([` IPureStringishObject `](/apis/Interfaces/IPureStringishObject/))




+ [` ->__toString(): string `](/apis/Interfaces/IPureStringishObject/__toString/)







### Public Methods ([` ConstIndexAccess `](/apis/Interfaces/ConstIndexAccess/))




* [` ->at(Tk $k): Tv `](/apis/Interfaces/ConstIndexAccess/at/)\
  Returns the value at the specified key in the current collection
* [` ->containsKey(mixed $k): bool `](/apis/Interfaces/ConstIndexAccess/containsKey/)\
  Determines if the specified key is in the current collection
* [` ->get(Tk $k): ?Tv `](/apis/Interfaces/ConstIndexAccess/get/)\
  Returns the value at the specified key in the current collection







### Public Methods ([` HH\KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/))




- [` ->getIterator(): KeyedIterator<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/getIterator/)\
  Returns an iterator that points to beginning of the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)
- [` ->lazy(): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)
- [` ->toImmMap(): ImmMap<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/toImmMap/)\
  Returns an immutable map ([` ImmMap `](/apis/Classes/HH/ImmMap/)) based on the keys and values of the
  current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)
- [` ->toKeysArray(): varray `](/apis/Interfaces/HH/KeyedIterable/toKeysArray/)\
  Returns an `` array `` with the keys from the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)







### Public Methods ([` HH\Iterable `](/apis/Interfaces/HH/Iterable/))




+ [` ->toImmSet(): ImmSet<Tv> `](/apis/Interfaces/HH/Iterable/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/apis/Classes/HH/ImmSet/)) converted from the current [` Iterable `](/apis/Interfaces/HH/Iterable/)
+ [` ->toImmVector(): ImmVector<Tv> `](/apis/Interfaces/HH/Iterable/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) converted from the current
  [` Iterable `](/apis/Interfaces/HH/Iterable/)
+ [` ->toValuesArray(): varray<Tv> `](/apis/Interfaces/HH/Iterable/toValuesArray/)\
  Returns an `` array `` with the values from the current [` Iterable `](/apis/Interfaces/HH/Iterable/)
<!-- HHAPIDOC -->
