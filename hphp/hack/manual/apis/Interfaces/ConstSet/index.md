---
title: ConstSet
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a read-only (immutable) set of values, with no keys
(i.e., a set)




## Guides




+ [Introduction](</docs/hack/arrays-and-collections/introduction>)
+ [Interfaces](</docs/hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface ConstSet implements ConstCollection<Tv>, ConstSetAccess<Tv>, HH\KeyedIterable<arraykey, Tv>, HH\KeyedContainer<Tv, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): ConstVector<Tu> `](/docs/apis/Interfaces/ConstSet/concat/)\
  Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) that is the concatenation of the values of the
  current `` ConstSet `` and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): ConstSet<Tv> `](/docs/apis/Interfaces/ConstSet/filter/)\
  Returns a `` ConstSet `` containing the values of the current ``` ConstSet ``` that
  meet a supplied condition applied to each value
* [` ->filterWithKey((function(arraykey, Tv): bool) $fn): ConstSet<Tv> `](/docs/apis/Interfaces/ConstSet/filterWithKey/)\
  Returns a `` ConstSet `` containing the values of the current ``` ConstSet ``` that
  meet a supplied condition applied to its "keys" and values
* [` ->firstKey(): ?arraykey `](/docs/apis/Interfaces/ConstSet/firstKey/)\
  Returns the first "key" in the current `` ConstSet ``
* [` ->firstValue(): ?Tv `](/docs/apis/Interfaces/ConstSet/firstValue/)\
  Returns the first value in the current `` ConstSet ``
* [` ->keys(): ConstVector<arraykey> `](/docs/apis/Interfaces/ConstSet/keys/)\
  Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the values of the current `` ConstSet ``
* [` ->lastKey(): ?arraykey `](/docs/apis/Interfaces/ConstSet/lastKey/)\
  Returns the last "key" in the current `` ConstSet ``
* [` ->lastValue(): ?Tv `](/docs/apis/Interfaces/ConstSet/lastValue/)\
  Returns the last value in the current `` ConstSet ``
* [` ->map<Tu as arraykey>((function(Tv): Tu) $fn): ConstSet<Tu> `](/docs/apis/Interfaces/ConstSet/map/)\
  Returns a `` ConstSet `` containing the values after an operation has been
  applied to each value in the current ``` ConstSet ```
* [` ->mapWithKey<Tu as arraykey>((function(arraykey, Tv): Tu) $fn): ConstSet<Tu> `](/docs/apis/Interfaces/ConstSet/mapWithKey/)\
  Returns a `` ConstSet `` containing the values after an operation has been
  applied to each "key" and value in the current Set
* [` ->skip(int $n): ConstSet<Tv> `](/docs/apis/Interfaces/ConstSet/skip/)\
  Returns a `` ConstSet `` containing the values after the ``` n ```-th element of the
  current ```` ConstSet ````
* [` ->skipWhile((function(Tv): bool) $fn): ConstSet<Tv> `](/docs/apis/Interfaces/ConstSet/skipWhile/)\
  Returns a `` ConstSet `` containing the values of the current ``` ConstSet ```
  starting after and including the first value that produces ```` true ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): ConstSet<Tv> `](/docs/apis/Interfaces/ConstSet/slice/)\
  Returns a subset of the current `` ConstSet `` starting from a given key up
  to, but not including, the element at the provided length from the
  starting key
* [` ->take(int $n): ConstSet<Tv> `](/docs/apis/Interfaces/ConstSet/take/)\
  Returns a `` ConstSet `` containing the first ``` n ``` values of the current
  ```` ConstSet ````
* [` ->takeWhile((function(Tv): bool) $fn): ConstSet<Tv> `](/docs/apis/Interfaces/ConstSet/takeWhile/)\
  Returns a `` ConstSet `` containing the values of the current ``` ConstSet ``` up to
  but not including the first value that produces ```` false ```` when passed to the
  specified callback
* [` ->toDArray(): darray<Tv, Tv> `](/docs/apis/Interfaces/ConstSet/toDArray/)
* [` ->toVArray(): varray<Tv> `](/docs/apis/Interfaces/ConstSet/toVArray/)
* [` ->values(): ConstVector<Tv> `](/docs/apis/Interfaces/ConstSet/values/)\
  Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the values of the current `` ConstSet ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): ConstSet<nothing> `](/docs/apis/Interfaces/ConstSet/zip/)\
  Returns a `` ConstSet `` where each value is a [` Pair `](/docs/apis/Classes/HH/Pair/) that combines the value
  of the current `` ConstSet `` and the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)







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
<!-- HHAPIDOC -->
