---
title: ImmVector
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` ImmVector ` is an immutable `` Vector ``




HHVM provides a native implementation
for this class. The PHP class definition below is not actually used at run
time; it is simply provided for the typechecker and for developer reference.




An ` ImmVector ` cannot be mutated. No elements can be added to it or removed
from it, nor can elements be overwritten using assignment (i.e. `` $c[$k] = $v ``
is not allowed).




```
$v = Vector {1, 2, 3};
$fv = $v->toImmVector();
```




construct it with a [` Traversable `](/docs/apis/Interfaces/HH/Traversable/):




```
$a = vec[1, 2, 3];
$fv = new ImmVector($a);
```




or use the literal syntax:




```
$fv = ImmVector {1, 2, 3};
```




## Guides




+ [Introduction](</docs/hack/arrays-and-collections/introduction>)
+ [Classes](</docs/hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

final class ImmVector implements \ConstVector<Tv> {...}
```




### Public Methods




* [` ::fromItems(?Traversable<Tv> $iterable): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/fromItems/)\
  Creates an `` ImmVector `` from the given [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), or an empty
  `` ImmVector `` if ``` null ``` is passed
* [` ::fromKeysOf<Tk as arraykey>(?KeyedContainer<Tk, mixed> $container): ImmVector<Tk> `](/docs/apis/Classes/HH/ImmVector/fromKeysOf/)\
  Creates an `` ImmVector `` from the keys of the specified container
* [` ->__construct(?Traversable<Tv> $iterable = NULL): void `](/docs/apis/Classes/HH/ImmVector/__construct/)\
  Creates an `` ImmVector `` from the given [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), or an empty
  `` ImmVector `` if ``` null ``` is passed
* [` ->__toString(): string `](/docs/apis/Classes/HH/ImmVector/__toString/)\
  Returns the `` string `` version of the current ``` ImmVector ```, which is
  ```` "ImmVector" ````
* [` ->at(int $key): Tv `](/docs/apis/Classes/HH/ImmVector/at/)\
  Returns the value at the specified key in the current `` ImmVector ``
* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): ImmVector<Tu> `](/docs/apis/Classes/HH/ImmVector/concat/)\
  Returns an `` ImmVector `` that is the concatenation of the values of the
  current ``` ImmVector ``` and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)
* [` ->containsKey(mixed $key): bool `](/docs/apis/Classes/HH/ImmVector/containsKey/)\
  Determines if the specified key is in the current `` ImmVector ``
* [` ->count(): int `](/docs/apis/Classes/HH/ImmVector/count/)\
  Returns the number of elements in the current `` ImmVector ``
* [` ->filter((function(Tv): bool) $callback): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/filter/)\
  Returns a `` ImmVector `` containing the values of the current ``` ImmVector ``` that
  meet a supplied condition
* [` ->filterWithKey((function(int, Tv): bool) $callback): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/filterWithKey/)\
  Returns an `` ImmVector `` containing the values of the current ``` ImmVector ```
  that meet a supplied condition applied to its keys and values
* [` ->firstKey(): ?int `](/docs/apis/Classes/HH/ImmVector/firstKey/)\
  Returns the first key in the current `` ImmVector ``
* [` ->firstValue(): ?Tv `](/docs/apis/Classes/HH/ImmVector/firstValue/)\
  Returns the first value in the current `` ImmVector ``
* [` ->get(int $key): ?Tv `](/docs/apis/Classes/HH/ImmVector/get/)\
  Returns the value at the specified key in the current `` ImmVector ``
* [` ->getIterator(): KeyedIterator<int, Tv> `](/docs/apis/Classes/HH/ImmVector/getIterator/)\
  Returns an iterator that points to beginning of the current `` ImmVector ``
* [` ->immutable(): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/immutable/)\
  Returns the current `` ImmVector ``
* [` ->isEmpty(): bool `](/docs/apis/Classes/HH/ImmVector/isEmpty/)\
  Checks if the current `` ImmVector `` is empty
* [` ->items(): Iterable<Tv> `](/docs/apis/Classes/HH/ImmVector/items/)\
  Returns an [` Iterable `](/docs/apis/Interfaces/HH/Iterable/) view of the current `` ImmVector ``
* [` ->keys(): ImmVector<int> `](/docs/apis/Classes/HH/ImmVector/keys/)\
  Returns an `` ImmVector `` containing the keys, as values, of the current
  ``` ImmVector ```
* [` ->lastKey(): ?int `](/docs/apis/Classes/HH/ImmVector/lastKey/)\
  Returns the last key in the current `` ImmVector ``
* [` ->lastValue(): ?Tv `](/docs/apis/Classes/HH/ImmVector/lastValue/)\
  Returns the last value in the current `` ImmVector ``
* [` ->lazy(): KeyedIterable<int, Tv> `](/docs/apis/Classes/HH/ImmVector/lazy/)\
  Returns a lazy, access-elements-only-when-needed view of the current
  `` ImmVector ``
* [` ->linearSearch(mixed $search_value): int `](/docs/apis/Classes/HH/ImmVector/linearSearch/)\
  Returns the index of the first element that matches the search value
* [` ->map<Tu>((function(Tv): Tu) $callback): ImmVector<Tu> `](/docs/apis/Classes/HH/ImmVector/map/)\
  Returns an `` ImmVector `` containing the results of applying an operation to
  each value in the current ``` ImmVector ```
* [` ->mapWithKey<Tu>((function(int, Tv): Tu) $callback): ImmVector<Tu> `](/docs/apis/Classes/HH/ImmVector/mapWithKey/)\
  Returns an `` ImmVector `` containing the results of applying an operation to
  each key/value pair in the current ``` ImmVector ```
* [` ->skip(int $n): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/skip/)\
  Returns an `` ImmVector `` containing the values after the ``` $n ```-th element of
  the current ```` ImmVector ````
* [` ->skipWhile((function(Tv): bool) $fn): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/skipWhile/)\
  Returns an `` ImmVector `` containing the values of the current ``` ImmVector ```
  starting after and including the first value that produces ```` false ```` when
  passed to the specified callback
* [` ->slice(int $start, int $len): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/slice/)\
  Returns a subset of the current `` ImmVector `` starting from a given key up
  to, but not including, the element at the provided length from the
  starting key
* [` ->take(int $n): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/take/)\
  Returns an `` ImmVector `` containing the first ``` $n ``` values of the current
  ```` ImmVector ````
* [` ->takeWhile((function(Tv): bool) $callback): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/takeWhile/)\
  Returns an `` ImmVector `` containing the values of the current ``` ImmVector ``` up
  to but not including the first value that produces ```` false ```` when passed to
  the specified callback
* [` ->toDArray(): darray<int, Tv> `](/docs/apis/Classes/HH/ImmVector/toDArray/)
* [` ->toImmMap(): ImmMap<int, Tv> `](/docs/apis/Classes/HH/ImmVector/toImmMap/)\
  Returns an immutable integer-keyed Map ([` ImmMap `](/docs/apis/Classes/HH/ImmMap/)) based on the elements of
  the current `` ImmVector ``
* [` ->toImmSet(): ImmSet<Tv> `](/docs/apis/Classes/HH/ImmVector/toImmSet/)\
  Returns an immutable Set ([` ImmSet `](/docs/apis/Classes/HH/ImmSet/)) with the values of the current
  `` ImmVector ``
* [` ->toImmVector(): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/)\
  Returns the current `` ImmVector ``
* [` ->toKeysArray(): varray<Tv> `](/docs/apis/Classes/HH/ImmVector/toKeysArray/)\
  Returns an `` array `` whose values are the keys from the current ``` ImmVector ```
* [` ->toMap(): Map `](/docs/apis/Classes/HH/ImmVector/toMap/)\
  Returns a Map built from the keys and values of this ImmVector
* [` ->toSet(): Set `](/docs/apis/Classes/HH/ImmVector/toSet/)\
  Returns a Set built from the values of this ImmVector
* [` ->toVArray(): varray<Tv> `](/docs/apis/Classes/HH/ImmVector/toVArray/)\
  Returns a varray built from the values from this ImmVector
* [` ->toValuesArray(): varray<Tv> `](/docs/apis/Classes/HH/ImmVector/toValuesArray/)\
  Returns an `` array `` containing the values from the current ``` ImmVector ```
* [` ->toVector(): Vector `](/docs/apis/Classes/HH/ImmVector/toVector/)\
  Returns a Vector built from the values of this ImmVector
* [` ->values(): ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/values/)\
  Returns a new `` ImmVector `` containing the values of the current ``` ImmVector ```;
  that is, a copy of the current ```` ImmVector ````
* [` ->zip<Tu>(Traversable<Tu> $traversable): ImmVector<Pair<Tv, Tu>> `](/docs/apis/Classes/HH/ImmVector/zip/)\
  Returns an `` ImmVector `` where each element is a [` Pair `](/docs/apis/Classes/HH/Pair/) that combines the
  element of the current `` ImmVector `` and the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)
<!-- HHAPIDOC -->
