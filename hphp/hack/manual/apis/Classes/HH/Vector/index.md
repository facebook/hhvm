---
title: Vector
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` Vector ` is a stack-like collection




HHVM provides a native implementation
for this class. The PHP class definition below is not actually used at run
time; it is simply provided for the typechecker and for developer reference.




Like all objects in PHP, ` Vector `s have reference-like semantics. When a
caller passes a `` Vector `` to a callee, the callee can modify the ``` Vector ``` and
the caller will see the changes. ```` Vector ````s do not have "copy-on-write"
semantics.




` Vector `s only support integer keys. If a non-integer key is used, an
exception will be thrown.




` Vector `s support `` $m[$k] `` style syntax for getting and setting values by
key. ``` Vector ```s also support ```` isset($m[$k]) ```` and ````` empty($m[$k]) ````` syntax, and
they provide similar semantics as arrays. Elements can be added to a
`````` Vector `````` using ``````` $m[] = .. ``````` syntax.




` Vector `s do not support iterating while new elements are being added or
elements are being removed. When a new element is added or removed, all
iterators that point to the `` Vector `` shall be considered invalid.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Classes](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

final class Vector implements \MutableVector<Tv> {...}
```




### Public Methods




* [` ::fromArray(darray<arraykey, Tv> $arr): Vector<Tv> `](/apis/Classes/HH/Vector/fromArray/)\
  Returns a `` Vector `` containing the values from the specified ``` array ```
* [` ::fromItems(?Traversable<Tv> $iterable): Vector<Tv> `](/apis/Classes/HH/Vector/fromItems/)\
  Creates a `` Vector `` from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty `` Vector `` if
  ``` null ``` is passed
* [` ::fromKeysOf<Tk as arraykey>(?KeyedContainer<Tk, mixed> $container): Vector<Tk> `](/apis/Classes/HH/Vector/fromKeysOf/)\
  Creates a `` Vector `` from the keys of the specified container
* [` ->__construct(?Traversable<Tv> $iterable = NULL): void `](/apis/Classes/HH/Vector/__construct/)\
  Creates a `` Vector `` from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty `` Vector ``
  if ``` null ``` is passed
* [` ->__toString(): string `](/apis/Classes/HH/Vector/__toString/)\
  Returns the `` string `` version of the current ``` Vector ```, which is ```` "Vector" ````
* [` ->add(Tv $value): Vector<Tv> `](/apis/Classes/HH/Vector/add/)\
  Appends a value to the end of the current `` Vector ``, assigning it the next
  available integer key
* [` ->addAll(?Traversable<Tv> $iterable): Vector<Tv> `](/apis/Classes/HH/Vector/addAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), append a value into this
  `` Vector ``, assigning the next available integer key for each
* [` ->addAllKeysOf(?KeyedContainer<Tv, mixed> $container): Vector<Tv> `](/apis/Classes/HH/Vector/addAllKeysOf/)\
  Adds the keys of the specified container to the current `` Vector ``
* [` ->append(mixed $value): this `](/apis/Classes/HH/Vector/append/)
* [` ->at(int $key): Tv `](/apis/Classes/HH/Vector/at/)\
  Returns the value at the specified key in the current `` Vector ``
* [` ->clear(): Vector<Tv> `](/apis/Classes/HH/Vector/clear/)\
  Removes all the elements from the current `` Vector ``
* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): Vector<Tu> `](/apis/Classes/HH/Vector/concat/)\
  Returns a `` Vector `` that is the concatenation of the values of the current
  ``` Vector ``` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->contains(mixed $key): bool `](/apis/Classes/HH/Vector/contains/)\
  Returns true if the specified key is present in the Vector, returns false
  otherwise
* [` ->containsKey(mixed $key): bool `](/apis/Classes/HH/Vector/containsKey/)\
  Determines if the specified key is in the current `` Vector ``
* [` ->count(): int `](/apis/Classes/HH/Vector/count/)\
  Returns the number of elements in the current `` Vector ``
* [` ->filter((function(Tv): bool) $callback): Vector<Tv> `](/apis/Classes/HH/Vector/filter/)\
  Returns a `` Vector `` containing the values of the current ``` Vector ``` that meet
  a supplied condition
* [` ->filterWithKey((function(int, Tv): bool) $callback): Vector<Tv> `](/apis/Classes/HH/Vector/filterWithKey/)\
  Returns a `` Vector `` containing the values of the current ``` Vector ``` that meet
  a supplied condition applied to its keys and values
* [` ->firstKey(): ?int `](/apis/Classes/HH/Vector/firstKey/)\
  Returns the first key in the current `` Vector ``
* [` ->firstValue(): ?Tv `](/apis/Classes/HH/Vector/firstValue/)\
  Returns the first value in the current `` Vector ``
* [` ->get(int $key): ?Tv `](/apis/Classes/HH/Vector/get/)\
  Returns the value at the specified key in the current `` Vector ``
* [` ->getIterator(): KeyedIterator<int, Tv> `](/apis/Classes/HH/Vector/getIterator/)\
  Returns an iterator that points to beginning of the current `` Vector ``
* [` ->immutable(): ImmVector<Tv> `](/apis/Classes/HH/Vector/immutable/)\
  Returns an immutable copy ([` ImmVector `](/apis/Classes/HH/ImmVector/)) of the current `` Vector ``
* [` ->isEmpty(): bool `](/apis/Classes/HH/Vector/isEmpty/)\
  Checks if the current `` Vector `` is empty
* [` ->items(): Iterable<Tv> `](/apis/Classes/HH/Vector/items/)\
  Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) view of the current `` Vector ``
* [` ->keys(): Vector<int> `](/apis/Classes/HH/Vector/keys/)\
  Returns a `` Vector `` containing the keys of the current ``` Vector ```
* [` ->lastKey(): ?int `](/apis/Classes/HH/Vector/lastKey/)\
  Returns the last key in the current `` Vector ``
* [` ->lastValue(): ?Tv `](/apis/Classes/HH/Vector/lastValue/)\
  Returns the last value in the current `` Vector ``
* [` ->lazy(): KeyedIterable<int, Tv> `](/apis/Classes/HH/Vector/lazy/)\
  Returns a lazy, access-elements-only-when-needed view of the current
  `` Vector ``
* [` ->linearSearch(mixed $search_value): int `](/apis/Classes/HH/Vector/linearSearch/)\
  Returns the index of the first element that matches the search value
* [` ->map<Tu>((function(Tv): Tu) $callback): Vector<Tu> `](/apis/Classes/HH/Vector/map/)\
  Returns a `` Vector `` containing the results of applying an operation to each
  value in the current ``` Vector ```
* [` ->mapWithKey<Tu>((function(int, Tv): Tu) $callback): Vector<Tu> `](/apis/Classes/HH/Vector/mapWithKey/)\
  Returns a `` Vector `` containing the results of applying an operation to each
  key/value pair in the current ``` Vector ```
* [` ->pop(): Tv `](/apis/Classes/HH/Vector/pop/)\
  Remove the last element of the current `` Vector `` and return it
* [` ->removeKey(int $key): Vector<Tv> `](/apis/Classes/HH/Vector/removeKey/)\
  Removes the key/value pair with the specified key from the current
  `` Vector ``
* [` ->reserve(int $sz): void `](/apis/Classes/HH/Vector/reserve/)\
  Reserves enough memory to accommodate a given number of elements
* [` ->resize(int $size, Tv $value): void `](/apis/Classes/HH/Vector/resize/)\
  Resize the current `` Vector ``
* [` ->reverse(): void `](/apis/Classes/HH/Vector/reverse/)\
  Reverse the elements of the current `` Vector `` in place
* [` ->set(int $key, Tv $value): Vector<Tv> `](/apis/Classes/HH/Vector/set/)\
  Stores a value into the current `` Vector `` with the specified key,
  overwriting the previous value associated with the key
* [` ->setAll(?KeyedTraversable<int, Tv> $iterable): Vector<Tv> `](/apis/Classes/HH/Vector/setAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), stores a value into the
  current `` Vector `` associated with each key, overwriting the previous value
  associated with the key
* [` ->shuffle(): void `](/apis/Classes/HH/Vector/shuffle/)\
  Shuffles the values of the current `` Vector `` randomly in place
* [` ->skip(int $n): Vector<Tv> `](/apis/Classes/HH/Vector/skip/)\
  Returns a `` Vector `` containing the values after the ``` $n ```-th element of the
  current ```` Vector ````
* [` ->skipWhile((function(Tv): bool) $fn): Vector<Tv> `](/apis/Classes/HH/Vector/skipWhile/)\
  Returns a `` Vector `` containing the values of the current ``` Vector ``` starting
  after and including the first value that produces ```` false ```` when passed to
  the specified callback
* [` ->slice(int $start, int $len): Vector<Tv> `](/apis/Classes/HH/Vector/slice/)\
  Returns a subset of the current `` Vector `` starting from a given key up to,
  but not including, the element at the provided length from the starting key
* [` ->splice(int $offset, ?int $len = NULL): void `](/apis/Classes/HH/Vector/splice/)\
  Splice the current `` Vector `` in place
* [` ->take(int $n): Vector<Tv> `](/apis/Classes/HH/Vector/take/)\
  Returns a `` Vector `` containing the first ``` $n ``` values of the current
  ```` Vector ````
* [` ->takeWhile((function(Tv): bool) $callback): Vector<Tv> `](/apis/Classes/HH/Vector/takeWhile/)\
  Returns a `` Vector `` containing the values of the current ``` Vector ``` up to but
  not including the first value that produces ```` false ```` when passed to the
  specified callback
* [` ->toDArray(): darray<int, Tv> `](/apis/Classes/HH/Vector/toDArray/)
* [` ->toImmMap(): ImmMap<int, Tv> `](/apis/Classes/HH/Vector/toImmMap/)\
  Returns an immutable, integer-keyed map ([` ImmMap `](/apis/Classes/HH/ImmMap/)) based on the values of
  the current `` Vector ``
* [` ->toImmSet(): ImmSet<Tv> `](/apis/Classes/HH/Vector/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/apis/Classes/HH/ImmSet/)) based on the values of the current
  `` Vector ``
* [` ->toImmVector(): ImmVector<Tv> `](/apis/Classes/HH/Vector/)\
  Returns an immutable copy ([` ImmVector `](/apis/Classes/HH/ImmVector/)) of the current `` Vector ``
* [` ->toKeysArray(): varray<int> `](/apis/Classes/HH/Vector/toKeysArray/)\
  Returns an `` array `` whose values are the keys from the current ``` Vector ```
* [` ->toMap(): Map<int, Tv> `](/apis/Classes/HH/Vector/toMap/)\
  Returns an integer-keyed [` Map `](/apis/Classes/HH/Map/) based on the values of the current `` Vector ``
* [` ->toSet(): Set<Tv> `](/apis/Classes/HH/Vector/toSet/)\
  Returns a [` Set `](/apis/Classes/HH/Set/) based on the values of the current `` Vector ``
* [` ->toVArray(): varray<Tv> `](/apis/Classes/HH/Vector/toVArray/)\
  Returns a varray built from the values from this Vector
* [` ->toValuesArray(): varray<Tv> `](/apis/Classes/HH/Vector/toValuesArray/)\
  Returns an `` array `` containing the values from the current ``` Vector ```
* [` ->toVector(): Vector<Tv> `](/apis/Classes/HH/Vector/)\
  Returns a copy of the current `` Vector ``
* [` ->values(): Vector<Tv> `](/apis/Classes/HH/Vector/values/)\
  Returns a `` Vector `` containing the values of the current ``` Vector ```
* [` ->zip<Tu>(Traversable<Tu> $traversable): Vector<Pair<Tv, Tu>> `](/apis/Classes/HH/Vector/zip/)\
  Returns a `` Vector `` where each element is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
  element of the current `` Vector `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
<!-- HHAPIDOC -->
