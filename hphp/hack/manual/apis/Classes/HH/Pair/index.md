---
title: Pair
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` Pair ` is an immutable, fixed-size collection with exactly two elements
(possibly of different types)




HHVM provides a native implementation for
this class.  The Hack class definition below is not actually used at run
time; it is simply provided for the typechecker and for developer reference.




Like all objects in PHP, ` Pair `s have reference-like semantics. The elements
of a `` Pair `` cannot be mutated (i.e. you can't assign to the elements of a
``` Pair ```) though ```` Pair ````s may contain mutable objects.




` Pair `s only support integer keys. If a non-integer key is used, an
exception will be thrown.




` Pair `s support `` $m[$k] `` style syntax for getting values by key. ``` Pair ```s
also support ```` isset($m[$k]) ```` and ````` empty($m[$k]) ````` syntax, and they provide
similar semantics as arrays.




` Pair ` keys are always 0 and 1, respectively.




You may notice that many methods affecting the instace of ` Pair ` return an
[` ImmVector `](/apis/Classes/HH/ImmVector/) -- `` Pair ``s are essentially backed by 2-element [` ImmVector `](/apis/Classes/HH/ImmVector/)s.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Classes](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

final class Pair implements \ConstVector<mixed> {...}
```




### Public Methods




* [` ->__toString(): string `](/apis/Classes/HH/Pair/__toString/)\
  Returns the `` string `` version of the current ``` Pair ```, which is ```` "Pair" ````
* [` ->at(int $key): mixed `](/apis/Classes/HH/Pair/at/)\
  Returns the value at the specified key in the current `` Pair ``
* [` ->concat<Tu super mixed>(Traversable<mixed, Tu> $traversable): ImmVector<mixed, Tu> `](/apis/Classes/HH/Pair/concat/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) that is the concatenation of the values of the
  current `` Pair `` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->containsKey<Tu super int>(Tu $key): bool `](/apis/Classes/HH/Pair/containsKey/)\
  Checks whether a provided key exists in the current `` Pair ``
* [` ->count(): int `](/apis/Classes/HH/Pair/count/)\
  Returns 2; a `` Pair `` always has two values
* [` ->filter((function(mixed): bool) $callback): ImmVector<mixed> `](/apis/Classes/HH/Pair/filter/)\
  Returns a [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current `` Pair `` that
  meet a supplied condition
* [` ->filterWithKey((function(int, mixed): bool) $callback): ImmVector<mixed> `](/apis/Classes/HH/Pair/filterWithKey/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current `` Pair `` that
  meet a supplied condition applied to its keys and values
* [` ->firstKey(): int `](/apis/Classes/HH/Pair/firstKey/)\
  Returns the first key in the current `` Pair ``
* [` ->firstValue(): Tv1 `](/apis/Classes/HH/Pair/firstValue/)\
  Returns the first value in the current `` Pair ``
* [` ->get(int $key): mixed `](/apis/Classes/HH/Pair/get/)\
  Returns the value at the specified key in the current `` Pair ``
* [` ->getIterator(): KeyedIterator<int, mixed> `](/apis/Classes/HH/Pair/getIterator/)\
  Returns an iterator that points to beginning of the current `` Pair ``
* [` ->immutable(): this `](/apis/Classes/HH/Pair/immutable/)\
  Returns an immutable version of this collection
* [` ->isEmpty(): bool `](/apis/Classes/HH/Pair/isEmpty/)\
  Returns `` false ``; a ``` Pair ``` cannot be empty
* [` ->items(): Iterable<mixed> `](/apis/Classes/HH/Pair/items/)\
  Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) view of the current `` Pair ``
* [` ->keys(): ImmVector<int> `](/apis/Classes/HH/Pair/keys/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) with the values being the keys of the current
  `` Pair ``
* [` ->lastKey(): int `](/apis/Classes/HH/Pair/lastKey/)\
  Returns the last key in the current `` Pair ``
* [` ->lastValue(): Tv2 `](/apis/Classes/HH/Pair/lastValue/)\
  Returns the last value in the current `` Pair ``
* [` ->lazy(): KeyedIterable<int, mixed> `](/apis/Classes/HH/Pair/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  `` Pair ``
* [` ->linearSearch<Tu super mixed>(mixed $search_value): int `](/apis/Classes/HH/Pair/linearSearch/)\
  Returns the index of the first element that matches the search value
* [` ->map<Tu>((function(mixed): Tu) $callback): ImmVector<Tu> `](/apis/Classes/HH/Pair/map/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after an operation has been
  applied to each value in the current `` Pair ``
* [` ->mapWithKey<Tu>((function(int, mixed): Tu) $callback): ImmVector<Tu> `](/apis/Classes/HH/Pair/mapWithKey/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after an operation has been
  applied to each key and value in the current `` Pair ``
* [` ->skip(int $n): ImmVector<mixed> `](/apis/Classes/HH/Pair/skip/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after the `` n ``-th element of
  the current ``` Pair ```
* [` ->skipWhile((function(mixed): bool) $callback): ImmVector<mixed> `](/apis/Classes/HH/Pair/skipWhile/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current `` Pair `` starting
  after and including the first value that produces ``` true ``` when passed to
  the specified callback
* [` ->slice(int $start, int $len): ImmVector<mixed> `](/apis/Classes/HH/Pair/slice/)\
  Returns a subset of the current `` Pair `` starting from a given key up to,
  but not including, the element at the provided length from the starting
  key
* [` ->take(int $n): ImmVector<mixed> `](/apis/Classes/HH/Pair/take/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the first `` n `` values of the current
  ``` Pair ```
* [` ->takeWhile((function(mixed): bool) $callback): ImmVector<mixed> `](/apis/Classes/HH/Pair/takeWhile/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current `` Pair `` up to
  but not including the first value that produces ``` false ``` when passed to the
  specified callback
* [` ->toDArray(): darray<int, mixed> `](/apis/Classes/HH/Pair/toDArray/)
* [` ->toImmMap(): ImmMap<int, mixed> `](/apis/Classes/HH/Pair/toImmMap/)\
  Returns an immutable, integer-keyed map ([` ImmMap `](/apis/Classes/HH/ImmMap/)) based on the elements of
  the current `` Pair ``
* [` ->toImmSet(): ImmSet<arraykey, mixed> `](/apis/Classes/HH/Pair/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/apis/Classes/HH/ImmSet/)) with the values of the current `` Pair ``
* [` ->toImmVector(): ImmVector<mixed> `](/apis/Classes/HH/Pair/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) containing the elements of the
  current `` Pair ``
* [` ->toKeysArray(): varray<int> `](/apis/Classes/HH/Pair/toKeysArray/)\
  Returns a `` varray `` whose values are the keys from the current ``` Pair ```
* [` ->toMap(): Map<int, mixed> `](/apis/Classes/HH/Pair/toMap/)\
  Returns an integer-keyed [` Map `](/apis/Classes/HH/Map/) based on the elements of the current `` Pair ``
* [` ->toSet(): Set<arraykey, mixed> `](/apis/Classes/HH/Pair/toSet/)\
  Returns a [` Set `](/apis/Classes/HH/Set/) with the values of the current `` Pair ``
* [` ->toVArray(): varray `](/apis/Classes/HH/Pair/toVArray/)
* [` ->toValuesArray<Tu>(): varray<Tu> `](/apis/Classes/HH/Pair/toValuesArray/)\
  Returns an `` varray `` containing the values from the current ``` Pair ```
* [` ->toVector(): Vector<mixed> `](/apis/Classes/HH/Pair/toVector/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the elements of the current `` Pair ``
* [` ->values(): ImmVector<mixed> `](/apis/Classes/HH/Pair/values/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current `` Pair ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): ImmVector<Pair<mixed, Tu>> `](/apis/Classes/HH/Pair/zip/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) where each element is a `` Pair `` that combines each
  element of the current ``` Pair ``` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
<!-- HHAPIDOC -->
