---
title: MutableVector
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a write-enabled (mutable) sequence of values, indexed by integers
(i.e., a vector)




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface MutableVector implements ConstVector<Tv>, HH\Collection<Tv>, IndexAccess<int, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): MutableVector<Tu> `](/apis/Interfaces/MutableVector/concat/)\
  Returns a `` MutableVector `` that is the concatenation of the values of the
  current ``` MutableVector ``` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): MutableVector<Tv> `](/apis/Interfaces/MutableVector/filter/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ``` that meet a supplied condition
* [` ->filterWithKey((function(int, Tv): bool) $fn): MutableVector<Tv> `](/apis/Interfaces/MutableVector/filterWithKey/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ``` that meet a supplied condition applied to its keys and
  values
* [` ->firstKey(): ?int `](/apis/Interfaces/MutableVector/firstKey/)\
  Returns the first key in the current `` MutableVector ``
* [` ->firstValue(): ?Tv `](/apis/Interfaces/MutableVector/firstValue/)\
  Returns the first value in the current `` MutableVector ``
* [` ->keys(): MutableVector<int> `](/apis/Interfaces/MutableVector/keys/)\
  Returns a `` MutableVector `` containing the keys of the current
  ``` MutableVector ```
* [` ->lastKey(): ?int `](/apis/Interfaces/MutableVector/lastKey/)\
  Returns the last key in the current `` MutableVector ``
* [` ->lastValue(): ?Tv `](/apis/Interfaces/MutableVector/lastValue/)\
  Returns the last value in the current `` MutableVector ``
* [` ->linearSearch(mixed $search_value): int `](/apis/Interfaces/MutableVector/linearSearch/)\
  Returns the index of the first element that matches the search value
* [` ->map<Tu>((function(Tv): Tu) $fn): MutableVector<Tu> `](/apis/Interfaces/MutableVector/map/)\
  Returns a `` MutableVector `` containing the values after an operation has been
  applied to each value in the current ``` MutableVector ```
* [` ->mapWithKey<Tu>((function(int, Tv): Tu) $fn): MutableVector<Tu> `](/apis/Interfaces/MutableVector/mapWithKey/)\
  Returns a `` MutableVector `` containing the values after an operation has been
  applied to each key and value in the current ``` MutableVector ```
* [` ->skip(int $n): MutableVector<Tv> `](/apis/Interfaces/MutableVector/skip/)\
  Returns a `` MutableVector `` containing the values after the ``` n ```-th element of
  the current ```` MutableVector ````
* [` ->skipWhile((function(Tv): bool) $fn): MutableVector<Tv> `](/apis/Interfaces/MutableVector/skipWhile/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ``` starting after and including the first value that produces
  ```` true ```` when passed to the specified callback
* [` ->slice(int $start, int $len): MutableVector<Tv> `](/apis/Interfaces/MutableVector/slice/)\
  Returns a subset of the current `` MutableVector `` starting from a given key
  up to, but not including, the element at the provided length from the
  starting key
* [` ->take(int $n): MutableVector<Tv> `](/apis/Interfaces/MutableVector/take/)\
  Returns a `` MutableVector `` containing the first ``` n ``` values of the current
  ```` MutableVector ````
* [` ->takeWhile((function(Tv): bool) $fn): MutableVector<Tv> `](/apis/Interfaces/MutableVector/takeWhile/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ``` up to but not including the first value that produces
  ```` false ```` when passed to the specified callback
* [` ->toDArray(): darray<int, Tv> `](/apis/Interfaces/MutableVector/toDArray/)
* [` ->toVArray(): varray<Tv> `](/apis/Interfaces/MutableVector/toVArray/)
* [` ->values(): MutableVector<Tv> `](/apis/Interfaces/MutableVector/values/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ```
* [` ->zip<Tu>(Traversable<Tu> $traversable): MutableVector<Pair<Tv, Tu>> `](/apis/Interfaces/MutableVector/zip/)\
  Returns a `` MutableVector `` where each element is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
  element of the current `` MutableVector `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)







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







### Public Methods ([` HH\Collection `](/apis/Interfaces/HH/Collection/))




* [` ->clear() `](/apis/Interfaces/HH/Collection/clear/)\
  Removes all items from the collection







### Public Methods ([` OutputCollection `](/apis/Interfaces/OutputCollection/))




- [` ->add(Te $e): this `](/apis/Interfaces/OutputCollection/add/)\
  Add a value to the collection and return the collection itself
- [` ->addAll(?Traversable<Te> $traversable): this `](/apis/Interfaces/OutputCollection/addAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), append a value into the
  current collection







### Public Methods ([` IndexAccess `](/apis/Interfaces/IndexAccess/))




+ [` ->removeKey(Tk $k): this `](/apis/Interfaces/IndexAccess/removeKey/)\
  Removes the specified key (and associated value) from the current
  collection
+ [` ->set(Tk $k, Tv $v): this `](/apis/Interfaces/IndexAccess/set/)\
  Stores a value into the current collection with the specified key,
  overwriting the previous value associated with the key
+ [` ->setAll(?KeyedTraversable<Tk, Tv> $traversable): this `](/apis/Interfaces/IndexAccess/setAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), stores a value into the
  current collection associated with each key, overwriting the previous value
  associated with the key
<!-- HHAPIDOC -->
