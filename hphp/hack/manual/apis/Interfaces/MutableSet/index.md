---
title: MutableSet
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a write-enabled (mutable) set of values, not indexed by keys
(i.e. a set)




## Guides




+ [Introduction](</docs/hack/arrays-and-collections/introduction>)
+ [Interfaces](</docs/hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface MutableSet implements ConstSet<Tv>, HH\Collection<Tv>, SetAccess<Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): MutableVector<Tu> `](/docs/apis/Interfaces/MutableSet/concat/)\
  Returns a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) that is the concatenation of the values of the
  current `` MutableSet `` and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): MutableSet<Tv> `](/docs/apis/Interfaces/MutableSet/filter/)\
  Returns a `` MutableSet `` containing the values of the current ``` MutableSet ```
  that meet a supplied condition applied to each value
* [` ->filterWithKey((function(arraykey, Tv): bool) $fn): MutableSet<Tv> `](/docs/apis/Interfaces/MutableSet/filterWithKey/)\
  Returns a `` MutableSet `` containing the values of the current ``` MutableSet ```
  that meet a supplied condition applied to its "keys" and values
* [` ->firstKey(): ?arraykey `](/docs/apis/Interfaces/MutableSet/firstKey/)\
  Returns the first "key" in the current `` MutableSet ``
* [` ->firstValue(): ?Tv `](/docs/apis/Interfaces/MutableSet/firstValue/)\
  Returns the first value in the current `` MutableSet ``
* [` ->keys(): MutableVector<arraykey> `](/docs/apis/Interfaces/MutableSet/keys/)\
  Returns a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) containing the values of the current
  `` MutableSet ``
* [` ->lastKey(): ?arraykey `](/docs/apis/Interfaces/MutableSet/lastKey/)\
  Returns the last "key" in the current `` MutableSet ``
* [` ->lastValue(): ?Tv `](/docs/apis/Interfaces/MutableSet/lastValue/)\
  Returns the last value in the current `` MutableSet ``
* [` ->map<Tu as arraykey>((function(Tv): Tu) $fn): MutableSet<Tu> `](/docs/apis/Interfaces/MutableSet/map/)\
  Returns a `` MutableSet `` containing the values after an operation has been
  applied to each value in the current ``` MutableSet ```
* [` ->mapWithKey<Tu as arraykey>((function(arraykey, Tv): Tu) $fn): MutableSet<Tu> `](/docs/apis/Interfaces/MutableSet/mapWithKey/)\
  Returns a `` MutableSet `` containing the values after an operation has been
  applied to each "key" and value in the current Set
* [` ->skip(int $n): MutableSet<Tv> `](/docs/apis/Interfaces/MutableSet/skip/)\
  Returns a `` MutableSet `` containing the values after the ``` n ```-th element of
  the current ```` MutableSet ````
* [` ->skipWhile((function(Tv): bool) $fn): MutableSet<Tv> `](/docs/apis/Interfaces/MutableSet/skipWhile/)\
  Returns a `` MutableSet `` containing the values of the current ``` MutableSet ```
  starting after and including the first value that produces ```` true ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): MutableSet<Tv> `](/docs/apis/Interfaces/MutableSet/slice/)\
  Returns a subset of the current `` MutableSet `` starting from a given key up
  to, but not including, the element at the provided length from the
  starting key
* [` ->take(int $n): MutableSet<Tv> `](/docs/apis/Interfaces/MutableSet/take/)\
  Returns a `` MutableSet `` containing the first ``` n ``` values of the current
  ```` MutableSet ````
* [` ->takeWhile((function(Tv): bool) $fn): MutableSet<Tv> `](/docs/apis/Interfaces/MutableSet/takeWhile/)\
  Returns a `` MutableSet `` containing the values of the current ``` MutableSet ```
  up to but not including the first value that produces ```` false ```` when passed
  to the specified callback
* [` ->toDArray(): darray<Tv, Tv> `](/docs/apis/Interfaces/MutableSet/toDArray/)
* [` ->toVArray(): varray<Tv> `](/docs/apis/Interfaces/MutableSet/toVArray/)
* [` ->values(): MutableVector<Tv> `](/docs/apis/Interfaces/MutableSet/values/)\
  Returns a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) containing the values of the current
  `` MutableSet ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): MutableSet<nothing> `](/docs/apis/Interfaces/MutableSet/zip/)\
  Returns a `` MutableSet `` where each value is a [` Pair `](/docs/apis/Classes/HH/Pair/) that combines the
  value of the current `` MutableSet `` and the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)







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
  Checks whether a value is in the current `` Set ``







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







### Public Methods ([` SetAccess `](/docs/apis/Interfaces/SetAccess/))




+ [` ->remove(Tm $m): this `](/docs/apis/Interfaces/SetAccess/remove/)\
  Removes the provided value from the current `` Set ``
<!-- HHAPIDOC -->
