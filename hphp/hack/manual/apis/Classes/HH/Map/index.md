---
title: Map
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` Map ` is an ordered dictionary-style collection




HHVM provides a native
implementation for this class. The PHP class definition below is not
actually used at run time; it is simply provided for the typechecker and for
developer reference.




Like all objects in PHP, ` Map `s have reference-like semantics. When a caller
passes a `` Map `` to a callee, the callee can modify the ``` Map ``` and the caller
will see the changes. ```` Map ````s do not have "copy-on-write" semantics.




` Map `s preserve insertion order of key/value pairs. When iterating over a
`` Map ``, the key/value pairs appear in the order they were inserted. Also,
``` Map ```s do not automagically convert integer-like ```` string ```` keys (ex. ````` "123" `````)
into integer keys.




` Map `s only support `` int `` keys and ``` string ``` keys. If a key of a different
type is used, an exception will be thrown.




` Map `s support `` $m[$k] `` style syntax for getting and setting values by key.
``` Map ```s also support ```` isset($m[$k]) ```` and ````` empty($m[$k]) ````` syntax, and they
provide similar semantics as arrays. Adding an element with square bracket
syntax `````` [] `````` is supported either by providing a key between the brackets or
a [` Pair `](/apis/Classes/HH/Pair/) on the right-hand side. e.g.,
`` $m[$k] = $v `` is supported
``` $m[] = Pair {$k, $v} ``` is supported
```` $m[] = $v ```` is not supported.




` Map `s do not support iterating while new keys are being added or elements
are being removed. When a new key is added or an element is removed, all
iterators that point to the `` Map `` shall be considered invalid.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Classes](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

final class Map implements \MutableMap<Tk, Tv> {...}
```




### Public Methods




* [` ::fromArray(darray<Tk, Tv> $arr): Map<Tk, Tv> `](/apis/Classes/HH/Map/fromArray/)\
  Returns a `` Map `` containing the key/value pairs from the specified ``` array ```
* [` ::fromItems(?Traversable<Pair<Tk, Tv>> $iterable): Map<Tk, Tv> `](/apis/Classes/HH/Map/fromItems/)\
  Creates a `` Map `` from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty `` Map `` if
  ``` null ``` is passed
* [` ->__construct(?KeyedTraversable<Tk, Tv> $iterable = NULL): void `](/apis/Classes/HH/Map/__construct/)\
  Creates a `` Map `` from the given [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/), or an empty `` Map `` if
  ``` null ``` is passed
* [` ->__toString(): string `](/apis/Classes/HH/Map/__toString/)\
  Returns the `` string `` version of the current ``` Map ```, which is ```` "Map" ````
* [` ->add(Pair<Tk, Tv> $val): Map<Tk, Tv> `](/apis/Classes/HH/Map/add/)\
  Add a key/value pair to the end of the current `` Map ``
* [` ->addAll(?Traversable<Pair<Tk, Tv>> $iterable): Map<Tk, Tv> `](/apis/Classes/HH/Map/addAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), add a key/value pair into
  the current `` Map ``
* [` ->at(Tk $key): Tv `](/apis/Classes/HH/Map/at/)\
  Returns the value at the specified key in the current `` Map ``
* [` ->clear(): Map<Tk, Tv> `](/apis/Classes/HH/Map/clear/)\
  Remove all the elements from the current `` Map ``
* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): Vector<Tu> `](/apis/Classes/HH/Map/concat/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) that is the concatenation of the values of the current
  `` Map `` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->contains(mixed $key): bool `](/apis/Classes/HH/Map/contains/)\
  Determines if the specified key is in the current `` Map ``
* [` ->containsKey(mixed $key): bool `](/apis/Classes/HH/Map/containsKey/)\
  Determines if the specified key is in the current `` Map ``
* [` ->count(): int `](/apis/Classes/HH/Map/count/)\
  Provides the number of elements in the current `` Map ``
* [` ->differenceByKey(KeyedTraversable<Tk, Tv> $traversable): Map<Tk, Tv> `](/apis/Classes/HH/Map/differenceByKey/)\
  Returns a new `` Map `` with the keys that are in the current ``` Map ```, but not
  in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)
* [` ->filter((function(Tv): bool) $callback): Map<Tk, Tv> `](/apis/Classes/HH/Map/filter/)\
  Returns a `` Map `` containing the values of the current ``` Map ``` that meet
  a supplied condition
* [` ->filterWithKey((function(Tk, Tv): bool) $callback): Map<Tk, Tv> `](/apis/Classes/HH/Map/filterWithKey/)\
  Returns a `` Map `` containing the values of the current ``` Map ``` that meet
  a supplied condition applied to its keys and values
* [` ->firstKey(): ?Tk `](/apis/Classes/HH/Map/firstKey/)\
  Returns the first key in the current `` Map ``
* [` ->firstValue(): ?Tv `](/apis/Classes/HH/Map/firstValue/)\
  Returns the first value in the current `` Map ``
* [` ->get(Tk $key): ?Tv `](/apis/Classes/HH/Map/get/)\
  Returns the value at the specified key in the current `` Map ``
* [` ->getIterator(): KeyedIterator<Tk, Tv> `](/apis/Classes/HH/Map/getIterator/)\
  Returns an iterator that points to beginning of the current `` Map ``
* [` ->immutable(): ImmMap<Tk, Tv> `](/apis/Classes/HH/Map/immutable/)\
  Returns a deep, immutable copy ([` ImmMap `](/apis/Classes/HH/ImmMap/)) of this `` Map ``
* [` ->isEmpty(): bool `](/apis/Classes/HH/Map/isEmpty/)\
  Checks if the current `` Map `` is empty
* [` ->items(): Iterable<Pair<Tk, Tv>> `](/apis/Classes/HH/Map/items/)\
  Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) view of the current `` Map ``
* [` ->keys(): Vector<Tk> `](/apis/Classes/HH/Map/keys/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the keys of the current `` Map ``
* [` ->lastKey(): ?Tk `](/apis/Classes/HH/Map/lastKey/)\
  Returns the last key in the current `` Map ``
* [` ->lastValue(): ?Tv `](/apis/Classes/HH/Map/lastValue/)\
  Returns the last value in the current `` Map ``
* [` ->lazy(): KeyedIterable<Tk, Tv> `](/apis/Classes/HH/Map/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  `` Map ``
* [` ->map<Tu>((function(Tv): Tu) $callback): Map<Tk, Tu> `](/apis/Classes/HH/Map/map/)\
  Returns a `` Map `` after an operation has been applied to each value in the
  current ``` Map ```
* [` ->mapWithKey<Tu>((function(Tk, Tv): Tu) $callback): Map<Tk, Tu> `](/apis/Classes/HH/Map/mapWithKey/)\
  Returns a `` Map `` after an operation has been applied to each key and
  value in the current ``` Map ```
* [` ->remove(Tk $key): Map<Tk, Tv> `](/apis/Classes/HH/Map/remove/)\
  Removes the specified key (and associated value) from the current `` Map ``
* [` ->removeKey(Tk $key): Map<Tk, Tv> `](/apis/Classes/HH/Map/removeKey/)\
  Removes the specified key (and associated value) from the current `` Map ``
* [` ->reserve(int $sz): void `](/apis/Classes/HH/Map/reserve/)\
  Reserves enough memory to accommodate a given number of elements
* [` ->retain(mixed $callback): Map `](/apis/Classes/HH/Map/retain/)\
  Ensures that this Map contains only keys/values for which the specified
  callback returns true when passed the value
* [` ->retainWithKey(mixed $callback): Map `](/apis/Classes/HH/Map/retainWithKey/)\
  Ensures that this Map contains only keys/values for which the specified
  callback returns true when passed the key and the value
* [` ->set(Tk $key, Tv $value): Map<Tk, Tv> `](/apis/Classes/HH/Map/set/)\
  Stores a value into the current `` Map `` with the specified key, overwriting
  the previous value associated with the key
* [` ->setAll(?KeyedTraversable<Tk, Tv> $iterable): Map<Tk, Tv> `](/apis/Classes/HH/Map/setAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), stores a value into the
  current `` Map `` associated with each key, overwriting the previous value
  associated with the key
* [` ->skip(int $n): Map<Tk, Tv> `](/apis/Classes/HH/Map/skip/)\
  Returns a `` Map `` containing the values after the ``` n ```-th element of the
  current ```` Map ````
* [` ->skipWhile((function(Tv): bool) $fn): Map<Tk, Tv> `](/apis/Classes/HH/Map/skipWhile/)\
  Returns a `` Map `` containing the values of the current ``` Map ``` starting after
  and including the first value that produces ```` true ```` when passed to the
  specified callback
* [` ->slice(int $start, int $len): Map<Tk, Tv> `](/apis/Classes/HH/Map/slice/)\
  Returns a subset of the current `` Map `` starting from a given key location
  up to, but not including, the element at the provided length from the
  starting key location
* [` ->take(int $n): Map<Tk, Tv> `](/apis/Classes/HH/Map/take/)\
  Returns a `` Map `` containing the first ``` n ``` key/values of the current ```` Map ````
* [` ->takeWhile((function(Tv): bool) $callback): Map<Tk, Tv> `](/apis/Classes/HH/Map/takeWhile/)\
  Returns a `` Map `` containing the keys and values of the current ``` Map ``` up to
  but not including the first value that produces ```` false ```` when passed to the
  specified callback
* [` ->toDArray(): darray<Tk, Tv> `](/apis/Classes/HH/Map/toDArray/)\
  Returns a darray built from the keys and values from this Map
* [` ->toImmMap(): ImmMap<Tk, Tv> `](/apis/Classes/HH/Map/)\
  Returns a deep, immutable copy ([` ImmMap `](/apis/Classes/HH/ImmMap/)) of the current `` Map ``
* [` ->toImmSet(): ImmSet<Tv> `](/apis/Classes/HH/Map/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/apis/Classes/HH/ImmSet/)) based on the values of the current
  `` Map ``
* [` ->toImmVector(): ImmVector<Tv> `](/apis/Classes/HH/Map/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) with the values of the current
  `` Map ``
* [` ->toKeysArray(): varray<Tk> `](/apis/Classes/HH/Map/toKeysArray/)\
  Returns an `` array `` whose values are the keys of the current ``` Map ```
* [` ->toMap(): Map<Tk, Tv> `](/apis/Classes/HH/Map/)\
  Returns a deep copy of the current `` Map ``
* [` ->toSet(): Set<Tv> `](/apis/Classes/HH/Map/toSet/)\
  Returns a [` Set `](/apis/Classes/HH/Set/) based on the values of the current `` Map ``
* [` ->toVArray(): varray<Tv> `](/apis/Classes/HH/Map/toVArray/)
* [` ->toValuesArray(): varray<Tv> `](/apis/Classes/HH/Map/toValuesArray/)\
  Returns an `` array `` containing the values from the current ``` Map ```
* [` ->toVector(): Vector<Tv> `](/apis/Classes/HH/Map/toVector/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) with the values of the current `` Map ``
* [` ->values(): Vector<Tv> `](/apis/Classes/HH/Map/values/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current `` Map ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): Map<Tk, Pair<Tv, Tu>> `](/apis/Classes/HH/Map/zip/)\
  Returns a `` Map `` where each value is a [` Pair `](/apis/Classes/HH/Pair/) that combines the value
  of the current `` Map `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
<!-- HHAPIDOC -->
