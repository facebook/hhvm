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




+ [Introduction](</docs/hack/arrays-and-collections/introduction>)
+ [Interfaces](</docs/hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface MutableVector implements ConstVector<Tv>, HH\Collection<Tv>, IndexAccess<int, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): MutableVector<Tu> `](/docs/apis/Interfaces/MutableVector/concat/)\
  Returns a `` MutableVector `` that is the concatenation of the values of the
  current ``` MutableVector ``` and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/filter/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ``` that meet a supplied condition
* [` ->filterWithKey((function(int, Tv): bool) $fn): MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/filterWithKey/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ``` that meet a supplied condition applied to its keys and
  values
* [` ->firstKey(): ?int `](/docs/apis/Interfaces/MutableVector/firstKey/)\
  Returns the first key in the current `` MutableVector ``
* [` ->firstValue(): ?Tv `](/docs/apis/Interfaces/MutableVector/firstValue/)\
  Returns the first value in the current `` MutableVector ``
* [` ->keys(): MutableVector<int> `](/docs/apis/Interfaces/MutableVector/keys/)\
  Returns a `` MutableVector `` containing the keys of the current
  ``` MutableVector ```
* [` ->lastKey(): ?int `](/docs/apis/Interfaces/MutableVector/lastKey/)\
  Returns the last key in the current `` MutableVector ``
* [` ->lastValue(): ?Tv `](/docs/apis/Interfaces/MutableVector/lastValue/)\
  Returns the last value in the current `` MutableVector ``
* [` ->linearSearch(mixed $search_value): int `](/docs/apis/Interfaces/MutableVector/linearSearch/)\
  Returns the index of the first element that matches the search value
* [` ->map<Tu>((function(Tv): Tu) $fn): MutableVector<Tu> `](/docs/apis/Interfaces/MutableVector/map/)\
  Returns a `` MutableVector `` containing the values after an operation has been
  applied to each value in the current ``` MutableVector ```
* [` ->mapWithKey<Tu>((function(int, Tv): Tu) $fn): MutableVector<Tu> `](/docs/apis/Interfaces/MutableVector/mapWithKey/)\
  Returns a `` MutableVector `` containing the values after an operation has been
  applied to each key and value in the current ``` MutableVector ```
* [` ->skip(int $n): MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/skip/)\
  Returns a `` MutableVector `` containing the values after the ``` n ```-th element of
  the current ```` MutableVector ````
* [` ->skipWhile((function(Tv): bool) $fn): MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/skipWhile/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ``` starting after and including the first value that produces
  ```` true ```` when passed to the specified callback
* [` ->slice(int $start, int $len): MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/slice/)\
  Returns a subset of the current `` MutableVector `` starting from a given key
  up to, but not including, the element at the provided length from the
  starting key
* [` ->take(int $n): MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/take/)\
  Returns a `` MutableVector `` containing the first ``` n ``` values of the current
  ```` MutableVector ````
* [` ->takeWhile((function(Tv): bool) $fn): MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/takeWhile/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ``` up to but not including the first value that produces
  ```` false ```` when passed to the specified callback
* [` ->toDArray(): darray<int, Tv> `](/docs/apis/Interfaces/MutableVector/toDArray/)
* [` ->toVArray(): varray<Tv> `](/docs/apis/Interfaces/MutableVector/toVArray/)
* [` ->values(): MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/values/)\
  Returns a `` MutableVector `` containing the values of the current
  ``` MutableVector ```
* [` ->zip<Tu>(Traversable<Tu> $traversable): MutableVector<Pair<Tv, Tu>> `](/docs/apis/Interfaces/MutableVector/zip/)\
  Returns a `` MutableVector `` where each element is a [` Pair `](/docs/apis/Classes/HH/Pair/) that combines the
  element of the current `` MutableVector `` and the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)







### Public Methods ([` ConstCollection `](/docs/apis/Interfaces/ConstCollection/))




- [` ->count(): int `](/docs/apis/Interfaces/ConstCollection/count/)\
  Get the number of items in the collection

- [` ->isEmpty(): bool `](/docs/apis/Interfaces/ConstCollection/isEmpty/)\
  Is the collection empty?

- [` ->items(): HH\Iterable<Te> `](/docs/apis/Interfaces/ConstCollection/items/)\
  Get access to the items in the collection








### Public Methods ([` IPureStringishObject `](/docs/apis/Interfaces/IPureStringishObject/))




+ [` ->__toString(): string `](/docs/apis/Interfaces/IPureStringishObject/__toString/)







### Public Methods ([` ConstIndexAccess `](/docs/apis/Interfaces/ConstIndexAccess/))




* [` ->at(Tk $k): Tv `](/docs/apis/Interfaces/ConstIndexAccess/at/)\
  Returns the value at the specified key in the current collection
* [` ->containsKey(mixed $k): bool `](/docs/apis/Interfaces/ConstIndexAccess/containsKey/)\
  Determines if the specified key is in the current collection
* [` ->get(Tk $k): ?Tv `](/docs/apis/Interfaces/ConstIndexAccess/get/)\
  Returns the value at the specified key in the current collection







### Public Methods ([` HH\KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/))




- [` ->getIterator(): KeyedIterator<Tk, Tv> `](/docs/apis/Interfaces/HH/KeyedIterable/getIterator/)\
  Returns an iterator that points to beginning of the current
  [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/)
- [` ->lazy(): KeyedIterable<Tk, Tv> `](/docs/apis/Interfaces/HH/KeyedIterable/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/)
- [` ->toImmMap(): ImmMap<Tk, Tv> `](/docs/apis/Interfaces/HH/KeyedIterable/toImmMap/)\
  Returns an immutable map ([` ImmMap `](/docs/apis/Classes/HH/ImmMap/)) based on the keys and values of the
  current [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/)
- [` ->toKeysArray(): varray `](/docs/apis/Interfaces/HH/KeyedIterable/toKeysArray/)\
  Returns an `` array `` with the keys from the current [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/)







### Public Methods ([` HH\Iterable `](/docs/apis/Interfaces/HH/Iterable/))




+ [` ->toImmSet(): ImmSet<Tv> `](/docs/apis/Interfaces/HH/Iterable/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/docs/apis/Classes/HH/ImmSet/)) converted from the current [` Iterable `](/docs/apis/Interfaces/HH/Iterable/)
+ [` ->toImmVector(): ImmVector<Tv> `](/docs/apis/Interfaces/HH/Iterable/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/docs/apis/Classes/HH/ImmVector/)) converted from the current
  [` Iterable `](/docs/apis/Interfaces/HH/Iterable/)
+ [` ->toValuesArray(): varray<Tv> `](/docs/apis/Interfaces/HH/Iterable/toValuesArray/)\
  Returns an `` array `` with the values from the current [` Iterable `](/docs/apis/Interfaces/HH/Iterable/)







### Public Methods ([` HH\Collection `](/docs/apis/Interfaces/HH/Collection/))




* [` ->clear() `](/docs/apis/Interfaces/HH/Collection/clear/)\
  Removes all items from the collection







### Public Methods ([` OutputCollection `](/docs/apis/Interfaces/OutputCollection/))




- [` ->add(Te $e): this `](/docs/apis/Interfaces/OutputCollection/add/)\
  Add a value to the collection and return the collection itself
- [` ->addAll(?Traversable<Te> $traversable): this `](/docs/apis/Interfaces/OutputCollection/addAll/)\
  For every element in the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), append a value into the
  current collection







### Public Methods ([` IndexAccess `](/docs/apis/Interfaces/IndexAccess/))




+ [` ->removeKey(Tk $k): this `](/docs/apis/Interfaces/IndexAccess/removeKey/)\
  Removes the specified key (and associated value) from the current
  collection
+ [` ->set(Tk $k, Tv $v): this `](/docs/apis/Interfaces/IndexAccess/set/)\
  Stores a value into the current collection with the specified key,
  overwriting the previous value associated with the key
+ [` ->setAll(?KeyedTraversable<Tk, Tv> $traversable): this `](/docs/apis/Interfaces/IndexAccess/setAll/)\
  For every element in the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), stores a value into the
  current collection associated with each key, overwriting the previous value
  associated with the key
<!-- HHAPIDOC -->
