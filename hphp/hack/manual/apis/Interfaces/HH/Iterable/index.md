---
title: Iterable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents any entity that can be iterated over using something like
` foreach `




The entity does not necessarily have to have a key, just values.




` Iterable ` does not include `` array ``s.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface Iterable implements Traversable<Tv>, \IteratorAggregate<Tv> {...}
```




### Public Methods




* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu> `](/apis/Interfaces/HH/Iterable/concat/)\
  Returns an `` Iterable `` that is the concatenation of the values of the
  current ``` Iterable ``` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->filter((function(Tv): bool) $fn): Iterable<Tv> `](/apis/Interfaces/HH/Iterable/filter/)\
  Returns an `` Iterable `` containing the values of the current ``` Iterable ``` that
  meet a supplied condition
* [` ->firstValue(): ?Tv `](/apis/Interfaces/HH/Iterable/firstValue/)\
  Returns the first value in the current `` Iterable ``
* [` ->getIterator(): Iterator<Tv> `](/apis/Interfaces/HH/Iterable/getIterator/)\
  Returns an iterator that points to beginning of the current `` Iterable ``
* [` ->lastValue(): ?Tv `](/apis/Interfaces/HH/Iterable/lastValue/)\
  Returns the last value in the current `` Iterable ``
* [` ->lazy(): Iterable<Tv> `](/apis/Interfaces/HH/Iterable/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  `` Iterable ``
* [` ->map<Tu>((function(Tv): Tu) $fn): Iterable<Tu> `](/apis/Interfaces/HH/Iterable/map/)\
  Returns an `` Iterable `` containing the values after an operation has been
  applied to each value in the current ``` Iterable ```
* [` ->skip(int $n): Iterable<Tv> `](/apis/Interfaces/HH/Iterable/skip/)\
  Returns an `` Iterable `` containing the values after the ``` n ```-th element of the
  current ```` Iterable ````
* [` ->skipWhile((function(Tv): bool) $fn): Iterable<Tv> `](/apis/Interfaces/HH/Iterable/skipWhile/)\
  Returns an `` Iterable `` containing the values of the current ``` Iterable ```
  starting after and including the first value that produces ```` true ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): Iterable<Tv> `](/apis/Interfaces/HH/Iterable/slice/)\
  Returns a subset of the current `` Iterable `` starting from a given key up
  to, but not including, the element at the provided length from the
  starting key
* [` ->take(int $n): Iterable<Tv> `](/apis/Interfaces/HH/Iterable/take/)\
  Returns an `` Iterable `` containing the first ``` n ``` values of the current
  ```` Iterable ````
* [` ->takeWhile((function(Tv): bool) $fn): Iterable<Tv> `](/apis/Interfaces/HH/Iterable/takeWhile/)\
  Returns an `` Iterable `` containing the values of the current ``` Iterable ``` up
  to but not including the first value that produces ```` false ```` when passed to
  the specified callback
* [` ->toImmSet(): ImmSet<Tv> `](/apis/Interfaces/HH/Iterable/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/apis/Classes/HH/ImmSet/)) converted from the current `` Iterable ``
* [` ->toImmVector(): ImmVector<Tv> `](/apis/Interfaces/HH/Iterable/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) converted from the current
  `` Iterable ``
* [` ->toValuesArray(): varray<Tv> `](/apis/Interfaces/HH/Iterable/toValuesArray/)\
  Returns an `` array `` with the values from the current ``` Iterable ```
* [` ->values(): Iterable<Tv> `](/apis/Interfaces/HH/Iterable/values/)\
  Returns an `` Iterable `` containing the current ``` Iterable ```'s values
* [` ->zip<Tu>(Traversable<Tu> $traversable): Iterable<Pair<Tv, Tu>> `](/apis/Interfaces/HH/Iterable/zip/)\
  Returns an `` Iterable `` where each element is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
  element of the current `` Iterable `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
<!-- HHAPIDOC -->
