---
title: Set
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` Set ` is an ordered set-style collection




HHVM provides a native
implementation for this class. The PHP class definition below is not
actually used at run time; it is simply provided for the typechecker and
for developer reference.




Like all objects in PHP, ` Set `s have reference-like semantics. When a caller
passes a `` Set `` to a callee, the callee can modify the ``` Set ``` and the caller
will see the changes. ```` Set ````s do not have "copy-on-write" semantics.




` Set `s preserve insertion order of the elements. When iterating over a
`` Set ``, the elements appear in the order they were inserted. Also, ``` Set ```s do
not automagically convert integer-like strings (ex. "123") into integers.




` Set `s only support `` int `` values and ``` string ``` values. If a value of a
different type is used, an exception will be thrown.




In general, Sets do not support ` $c[$k] ` style syntax. Adding an element
using `` $c[] = .. `` syntax is supported.




` Set ` do not support iteration while elements are being added or removed.
When an element is added or removed, all iterators that point to the `` Set ``
shall be considered invalid.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Classes](</hack/arrays-and-collections/introduction>)







## Examples




More documentation and examples are in the
[` Set ` and ` ImmSet `](</hack/arrays-and-collections/introduction>)
guide.







## Interface Synopsis




``` Hack
namespace HH;

final class Set implements \MutableSet<Tv> {...}
```




### Public Methods




* [` ::fromArray(darray<arraykey, Tv> $arr): Set<Tv> `](/apis/Classes/HH/Set/fromArray/)\
  Returns a `` Set `` containing the values from the specified ``` array ```
* [` ::fromArrays(...$argv): Set<Tv> `](/apis/Classes/HH/Set/fromArrays/)\
  Returns a `` Set `` containing all the values from the specified ``` array ```(s)
* [` ::fromItems(?Traversable<Tv> $iterable): Set<Tv> `](/apis/Classes/HH/Set/fromItems/)\
  Creates a `` Set `` from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty `` Set `` if ``` null ```
  is passed
* [` ::fromKeysOf<Tk as arraykey>(?KeyedContainer<Tk, mixed> $container): Set<Tk> `](/apis/Classes/HH/Set/fromKeysOf/)\
  Creates a `` Set `` from the keys of the specified container
* [` ->__construct(?Traversable<Tv> $iterable = NULL): void `](/apis/Classes/HH/Set/__construct/)\
  Creates a `` Set `` from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty `` Set `` if ``` null ```
  is passed
* [` ->__toString(): string `](/apis/Classes/HH/Set/__toString/)\
  Returns the `` string `` version of the current ``` Set ```, which is ```` "Set" ````
* [` ->add(Tv $val): Set<Tv> `](/apis/Classes/HH/Set/add/)\
  Add the value to the current `` Set ``
* [` ->addAll(?Traversable<Tv> $iterable): Set<Tv> `](/apis/Classes/HH/Set/addAll/)\
  For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), add the value into the
  current `` Set ``
* [` ->addAllKeysOf(?KeyedContainer<Tv, mixed> $container): Set<Tv> `](/apis/Classes/HH/Set/addAllKeysOf/)\
  Adds the keys of the specified container to the current `` Set `` as new
  values
* [` ->clear(): Set<Tv> `](/apis/Classes/HH/Set/clear/)\
  Remove all the elements from the current `` Set ``
* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): Vector<Tu> `](/apis/Classes/HH/Set/concat/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) that is the concatenation of the values of the current
  `` Set `` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->contains(arraykey $val): bool `](/apis/Classes/HH/Set/contains/)\
  Determines if the specified value is in the current `` Set ``
* [` ->count(): int `](/apis/Classes/HH/Set/count/)\
  Provides the number of elements in the current `` Set ``
* [` ->difference(mixed $iterable) `](/apis/Classes/HH/Set/difference/)
* [` ->filter((function(Tv): bool) $callback): Set<Tv> `](/apis/Classes/HH/Set/filter/)\
  Returns a `` Set `` containing the values of the current ``` Set ``` that meet
  a supplied condition applied to each value
* [` ->filterWithKey((function(arraykey, Tv): bool) $callback): Set<Tv> `](/apis/Classes/HH/Set/filterWithKey/)\
  Returns a `` Set `` containing the values of the current ``` Set ``` that meet
  a supplied condition applied to its "keys" and values
* [` ->firstKey(): ?arraykey `](/apis/Classes/HH/Set/firstKey/)\
  Returns the first "key" in the current `` Set ``
* [` ->firstValue(): ?Tv `](/apis/Classes/HH/Set/firstValue/)\
  Returns the first value in the current `` Set ``
* [` ->getIterator(): KeyedIterator<arraykey, Tv> `](/apis/Classes/HH/Set/getIterator/)\
  Returns an iterator that points to beginning of the current `` Set ``
* [` ->immutable(): ImmSet<Tv> `](/apis/Classes/HH/Set/immutable/)\
  Returns an immutable ([` ImmSet `](/apis/Classes/HH/ImmSet/)), deep copy of the current `` Set ``
* [` ->isEmpty(): bool `](/apis/Classes/HH/Set/isEmpty/)\
  Checks if the current `` Set `` is empty
* [` ->items(): Iterable<Tv> `](/apis/Classes/HH/Set/items/)\
  Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) view of the current `` Set ``
* [` ->keys(): Vector<arraykey> `](/apis/Classes/HH/Set/keys/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current `` Set ``
* [` ->lastKey(): ?arraykey `](/apis/Classes/HH/Set/lastKey/)\
  Returns the last "key" in the current `` Set ``
* [` ->lastValue(): ?Tv `](/apis/Classes/HH/Set/lastValue/)\
  Returns the last value in the current `` Set ``
* [` ->lazy(): KeyedIterable<arraykey, Tv> `](/apis/Classes/HH/Set/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  `` Set ``
* [` ->map<Tu as arraykey>((function(Tv): Tu) $callback): Set<Tu> `](/apis/Classes/HH/Set/map/)\
  Returns a `` Set `` containing the values after an operation has been applied
  to each value in the current ``` Set ```
* [` ->mapWithKey<Tu as arraykey>((function(arraykey, Tv): Tu) $callback): Set<Tu> `](/apis/Classes/HH/Set/mapWithKey/)\
  Returns a `` Set `` containing the values after an operation has been applied
  to each "key" and value in the current ``` Set ```
* [` ->remove(Tv $val): Set<Tv> `](/apis/Classes/HH/Set/remove/)\
  Removes the specified value from the current `` Set ``
* [` ->removeAll(Traversable<Tv> $iterable): Set<Tv> `](/apis/Classes/HH/Set/removeAll/)\
  Removes the values in the current `` Set `` that are also in the [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->reserve(int $sz): void `](/apis/Classes/HH/Set/reserve/)\
  Reserves enough memory to accommodate a given number of elements
* [` ->retain((function(Tv): bool) $callback): Set<Tv> `](/apis/Classes/HH/Set/retain/)\
  Alters the current `` Set `` so that it only contains the values that meet a
  supplied condition on each value
* [` ->retainWithKey((function(arraykey, Tv): bool) $callback): Set<Tv> `](/apis/Classes/HH/Set/retainWithKey/)\
  Alters the current `` Set `` so that it only contains the values that meet a
  supplied condition on its "keys" and values
* [` ->skip(int $n): Set<Tv> `](/apis/Classes/HH/Set/skip/)\
  Returns a `` Set `` containing the values after the ``` n ```-th element of the
  current ```` Set ````
* [` ->skipWhile((function(Tv): bool) $fn): Set<Tv> `](/apis/Classes/HH/Set/skipWhile/)\
  Returns a `` Set `` containing the values of the current ``` Set ``` starting after
  and including the first value that produces ```` true ```` when passed to the
  specified callback
* [` ->slice(int $start, int $len): Set<Tv> `](/apis/Classes/HH/Set/slice/)\
  Returns a subset of the current `` Set `` starting from a given key up to, but
  not including, the element at the provided length from the starting key
* [` ->take(int $n): Set<Tv> `](/apis/Classes/HH/Set/take/)\
  Returns a `` Set `` containing the first ``` n ``` values of the current ```` Set ````
* [` ->takeWhile((function(Tv): bool) $callback): Set<Tv> `](/apis/Classes/HH/Set/takeWhile/)\
  Returns a `` Set `` containing the values of the current ``` Set ``` up to but not
  including the first value that produces ```` false ```` when passed to the
  specified callback
* [` ->toDArray(): darray<Tv, Tv> `](/apis/Classes/HH/Set/toDArray/)\
  Returns a darray built from the values from this Set, darray[val1 => val1,
  val2 => val2, ...]
* [` ->toImmMap(): ImmMap<arraykey, Tv> `](/apis/Classes/HH/Set/toImmMap/)\
  Returns an immutable map ([` ImmMap `](/apis/Classes/HH/ImmMap/)) based on the values of the current
  `` Set ``
* [` ->toImmSet(): ImmSet<Tv> `](/apis/Classes/HH/Set/)\
  Returns an immutable ([` ImmSet `](/apis/Classes/HH/ImmSet/)), deep copy of the current `` Set ``
* [` ->toImmVector(): ImmVector<Tv> `](/apis/Classes/HH/Set/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) with the values of the current
  `` Set ``
* [` ->toKeysArray(): varray<Tv> `](/apis/Classes/HH/Set/toKeysArray/)\
  Returns an `` array `` containing the values from the current ``` Set ```
* [` ->toMap(): Map<arraykey, Tv> `](/apis/Classes/HH/Set/toMap/)\
  Returns a [` Map `](/apis/Classes/HH/Map/) based on the values of the current `` Set ``
* [` ->toSet(): Set<Tv> `](/apis/Classes/HH/Set/)\
  Returns a deep copy of the current `` Set ``
* [` ->toVArray(): varray<Tv> `](/apis/Classes/HH/Set/toVArray/)
* [` ->toValuesArray(): varray<Tv> `](/apis/Classes/HH/Set/toValuesArray/)\
  Returns an `` array `` containing the values from the current ``` Set ```
* [` ->toVector(): Vector<Tv> `](/apis/Classes/HH/Set/toVector/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) of the current `` Set `` values
* [` ->values(): Vector<Tv> `](/apis/Classes/HH/Set/values/)\
  Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current `` Set ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): Set<nothing> `](/apis/Classes/HH/Set/zip/)\
  Throws an exception unless the current `` Set `` or the [` Traversable `](/apis/Interfaces/HH/Traversable/) is
  empty
<!-- HHAPIDOC -->
