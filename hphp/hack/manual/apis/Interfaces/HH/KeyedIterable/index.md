---
title: KeyedIterable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents any entity that can be iterated over using something like
` foreach `




The entity is required to have a key in addition to values.




` KeyedIterable ` does not include `` array ``s.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface KeyedIterable implements Iterable<Tv>, KeyedTraversable<Tk, Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu> `](/apis/Interfaces/HH/KeyedIterable/concat/)\
  Returns an `` Iterable `` that is the concatenation of the values of the
  current ``` KeyedIterable ``` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/filter/)\
  Returns a `` KeyedIterable `` containing the values of the current
  ``` KeyedIterable ``` that meet a supplied condition
* [` ->filterWithKey((function(Tk, Tv): bool) $callback): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/filterWithKey/)\
  Returns a `` KeyedIterable `` containing the values of the current
  ``` KeyedIterable ``` that meet a supplied condition applied to its keys and
  values
* [` ->firstKey(): ?Tk `](/apis/Interfaces/HH/KeyedIterable/firstKey/)\
  Returns the first key in the current `` KeyedIterable ``
* [` ->firstValue(): ?Tv `](/apis/Interfaces/HH/KeyedIterable/firstValue/)\
  Returns the first value in the current `` KeyedIterable ``
* [` ->getIterator(): KeyedIterator<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/getIterator/)\
  Returns an iterator that points to beginning of the current
  `` KeyedIterable ``
* [` ->keys(): Iterable<Tk> `](/apis/Interfaces/HH/KeyedIterable/keys/)\
  Returns an `` Iterable `` containing the current ``` KeyedIterable ```'s keys
* [` ->lastKey(): ?Tk `](/apis/Interfaces/HH/KeyedIterable/lastKey/)\
  Returns the last key in the current `` KeyedIterable ``
* [` ->lastValue(): ?Tv `](/apis/Interfaces/HH/KeyedIterable/lastValue/)\
  Returns the last value in the current `` KeyedIterable ``
* [` ->lazy(): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  `` KeyedIterable ``
* [` ->map<Tu>((function(Tv): Tu) $fn): KeyedIterable<Tk, Tu> `](/apis/Interfaces/HH/KeyedIterable/map/)\
  Returns a `` KeyedIterable `` containing the values after an operation has been
  applied to each value in the current ``` KeyedIterable ```
* [` ->mapWithKey<Tu>((function(Tk, Tv): Tu) $callback): KeyedIterable<Tk, Tu> `](/apis/Interfaces/HH/KeyedIterable/mapWithKey/)\
  Returns a `` KeyedIterable `` containing the values after an operation has
  been applied to each key and value in the current ``` KeyedIterable ```
* [` ->skip(int $n): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/skip/)\
  Returns a `` KeyedIterable `` containing the values after the ``` n ```-th element
  of the current ```` KeyedIterable ````
* [` ->skipWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/skipWhile/)\
  Returns a `` KeyedIterable `` containing the values of the current
  ``` KeyedIterable ``` starting after and including the first value that produces
  ```` true ```` when passed to the specified callback
* [` ->slice(int $start, int $len): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/slice/)\
  Returns a subset of the current `` KeyedIterable `` starting from a given key
  up to, but not including, the element at the provided length from the
  starting key
* [` ->take(int $n): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/take/)\
  Returns a `` KeyedIterable `` containing the first ``` n ``` values of the current
  ```` KeyedIterable ````
* [` ->takeWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/takeWhile/)\
  Returns a `` KeyedIterable `` containing the values of the current
  ``` KeyedIterable ``` up to but not including the first value that produces
  ```` false ```` when passed to the specified callback
* [` ->toImmMap(): ImmMap<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/toImmMap/)\
  Returns an immutable map ([` ImmMap `](/apis/Classes/HH/ImmMap/)) based on the keys and values of the
  current `` KeyedIterable ``
* [` ->toKeysArray(): varray `](/apis/Interfaces/HH/KeyedIterable/toKeysArray/)\
  Returns an `` array `` with the keys from the current ``` KeyedIterable ```
* [` ->values(): Iterable<Tv> `](/apis/Interfaces/HH/KeyedIterable/values/)\
  Returns an `` Iterable `` containing the current ``` KeyedIterable ```'s values
* [` ->zip<Tu>(Traversable<Tu> $traversable): KeyedIterable<Tk, Pair<Tv, Tu>> `](/apis/Interfaces/HH/KeyedIterable/zip/)\
  Returns a `` KeyedIterable `` where each element is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
  element of the current `` KeyedIterable `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)







### Public Methods ([` HH\Iterable `](/apis/Interfaces/HH/Iterable/))




- [` ->toImmSet(): ImmSet<Tv> `](/apis/Interfaces/HH/Iterable/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/apis/Classes/HH/ImmSet/)) converted from the current `` Iterable ``
- [` ->toImmVector(): ImmVector<Tv> `](/apis/Interfaces/HH/Iterable/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) converted from the current
  `` Iterable ``
- [` ->toValuesArray(): varray<Tv> `](/apis/Interfaces/HH/Iterable/toValuesArray/)\
  Returns an `` array `` with the values from the current ``` Iterable ```
<!-- HHAPIDOC -->
