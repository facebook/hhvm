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




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
interface ConstSet implements ConstCollection<Tv>, ConstSetAccess<Tv>, HH\KeyedIterable<arraykey, Tv>, HH\KeyedContainer<Tv, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): ConstVector<Tu> `](/apis/Interfaces/ConstSet/concat/)\
  Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) that is the concatenation of the values of the
  current `` ConstSet `` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): ConstSet<Tv> `](/apis/Interfaces/ConstSet/filter/)\
  Returns a `` ConstSet `` containing the values of the current ``` ConstSet ``` that
  meet a supplied condition applied to each value
* [` ->filterWithKey((function(arraykey, Tv): bool) $fn): ConstSet<Tv> `](/apis/Interfaces/ConstSet/filterWithKey/)\
  Returns a `` ConstSet `` containing the values of the current ``` ConstSet ``` that
  meet a supplied condition applied to its "keys" and values
* [` ->firstKey(): ?arraykey `](/apis/Interfaces/ConstSet/firstKey/)\
  Returns the first "key" in the current `` ConstSet ``
* [` ->firstValue(): ?Tv `](/apis/Interfaces/ConstSet/firstValue/)\
  Returns the first value in the current `` ConstSet ``
* [` ->keys(): ConstVector<arraykey> `](/apis/Interfaces/ConstSet/keys/)\
  Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the values of the current `` ConstSet ``
* [` ->lastKey(): ?arraykey `](/apis/Interfaces/ConstSet/lastKey/)\
  Returns the last "key" in the current `` ConstSet ``
* [` ->lastValue(): ?Tv `](/apis/Interfaces/ConstSet/lastValue/)\
  Returns the last value in the current `` ConstSet ``
* [` ->map<Tu as arraykey>((function(Tv): Tu) $fn): ConstSet<Tu> `](/apis/Interfaces/ConstSet/map/)\
  Returns a `` ConstSet `` containing the values after an operation has been
  applied to each value in the current ``` ConstSet ```
* [` ->mapWithKey<Tu as arraykey>((function(arraykey, Tv): Tu) $fn): ConstSet<Tu> `](/apis/Interfaces/ConstSet/mapWithKey/)\
  Returns a `` ConstSet `` containing the values after an operation has been
  applied to each "key" and value in the current Set
* [` ->skip(int $n): ConstSet<Tv> `](/apis/Interfaces/ConstSet/skip/)\
  Returns a `` ConstSet `` containing the values after the ``` n ```-th element of the
  current ```` ConstSet ````
* [` ->skipWhile((function(Tv): bool) $fn): ConstSet<Tv> `](/apis/Interfaces/ConstSet/skipWhile/)\
  Returns a `` ConstSet `` containing the values of the current ``` ConstSet ```
  starting after and including the first value that produces ```` true ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): ConstSet<Tv> `](/apis/Interfaces/ConstSet/slice/)\
  Returns a subset of the current `` ConstSet `` starting from a given key up
  to, but not including, the element at the provided length from the
  starting key
* [` ->take(int $n): ConstSet<Tv> `](/apis/Interfaces/ConstSet/take/)\
  Returns a `` ConstSet `` containing the first ``` n ``` values of the current
  ```` ConstSet ````
* [` ->takeWhile((function(Tv): bool) $fn): ConstSet<Tv> `](/apis/Interfaces/ConstSet/takeWhile/)\
  Returns a `` ConstSet `` containing the values of the current ``` ConstSet ``` up to
  but not including the first value that produces ```` false ```` when passed to the
  specified callback
* [` ->toDArray(): darray<Tv, Tv> `](/apis/Interfaces/ConstSet/toDArray/)
* [` ->toVArray(): varray<Tv> `](/apis/Interfaces/ConstSet/toVArray/)
* [` ->values(): ConstVector<Tv> `](/apis/Interfaces/ConstSet/values/)\
  Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the values of the current `` ConstSet ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): ConstSet<nothing> `](/apis/Interfaces/ConstSet/zip/)\
  Returns a `` ConstSet `` where each value is a [` Pair `](/apis/Classes/HH/Pair/) that combines the value
  of the current `` ConstSet `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)







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
  Checks whether a value is in the current `` Set ``







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
