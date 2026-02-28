---
title: ImmSet
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` ImmSet ` is an immutable, ordered set-style collection




HHVM provides a
native implementation for this class. The PHP class definition below is not
actually used at run time; it is simply provided for the typechecker and
for developer reference.




An ` ImmSet ` cannot be mutated. No elements can be added or removed from it,
nor can elements be overwritten using assignment (i.e. `` $s[$k] = $v `` is
not allowed).




Construct it with a [` Traversable `](/apis/Interfaces/HH/Traversable/):




```
$a = vec[1, 2];
$s = new ImmSet($a);
```




or use the literal syntax:




```
$s = ImmSet {1, 2};
```




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Classes](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

final class ImmSet implements \ConstSet<Tv> {...}
```




### Public Methods




* [` ::fromArrays(...$argv): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/fromArrays/)\
  Returns an `` ImmSet `` containing all the values from the specified
  ``` array ```(s)
* [` ::fromItems(?Traversable<Tv> $iterable): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/fromItems/)\
  Creates an `` ImmSet `` from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty `` ImmSet `` if
  ``` null ``` is passed
* [` ::fromKeysOf<Tk as arraykey>(?KeyedContainer<Tk, mixed> $container): ImmSet<Tk> `](/apis/Classes/HH/ImmSet/fromKeysOf/)\
  Creates an `` ImmSet `` from the keys of the specified container
* [` ->__construct(?Traversable<Tv> $iterable = NULL): void `](/apis/Classes/HH/ImmSet/__construct/)\
  Creates an `` ImmSet `` from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty `` ImmSet `` if
  ``` null ``` is passed
* [` ->__toString(): string `](/apis/Classes/HH/ImmSet/__toString/)\
  Returns the `` string `` version of this ``` ImmSet ```, which is ```` "ImmSet" ````
* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): ImmVector<Tu> `](/apis/Classes/HH/ImmSet/concat/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) that is the concatenation of the values of the
  current `` ImmSet `` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->contains(arraykey $val): bool `](/apis/Classes/HH/ImmSet/contains/)\
  Determines if the specified value is in the current `` ImmSet ``
* [` ->count(): int `](/apis/Classes/HH/ImmSet/count/)\
  Provides the number of elements in the current `` ImmSet ``
* [` ->filter((function(Tv): bool) $callback): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/filter/)\
  Returns an `` ImmSet `` containing the values of the current ``` ImmSet ``` that
  meet a supplied condition applied to each value
* [` ->filterWithKey((function(arraykey, Tv): bool) $callback): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/filterWithKey/)\
  Returns an `` ImmSet `` containing the values of the current ``` ImmSet ``` that
  meet a supplied condition applied to its "keys" and values
* [` ->firstKey(): ?arraykey `](/apis/Classes/HH/ImmSet/firstKey/)\
  Returns the first "key" in the current `` ImmSet ``
* [` ->firstValue(): ?Tv `](/apis/Classes/HH/ImmSet/firstValue/)\
  Returns the first value in the current `` ImmSet ``
* [` ->getIterator(): KeyedIterator<arraykey, Tv> `](/apis/Classes/HH/ImmSet/getIterator/)\
  Returns an iterator that points to beginning of the current `` ImmSet ``
* [` ->immutable(): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/immutable/)\
  Returns an immutable copy (`` ImmSet ``) of the current ``` ImmSet ```
* [` ->isEmpty(): bool `](/apis/Classes/HH/ImmSet/isEmpty/)\
  Checks if the current `` ImmSet `` is empty
* [` ->items(): Iterable<Tv> `](/apis/Classes/HH/ImmSet/items/)\
  Returns an Iterable view of the current `` ImmSet ``
* [` ->keys(): ImmVector<arraykey> `](/apis/Classes/HH/ImmSet/keys/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of this `` ImmSet ``
* [` ->lastKey(): ?arraykey `](/apis/Classes/HH/ImmSet/lastKey/)\
  Returns the last "key" in the current `` ImmSet ``
* [` ->lastValue(): ?Tv `](/apis/Classes/HH/ImmSet/lastValue/)\
  Returns the last value in the current `` ImmSet ``
* [` ->lazy(): KeyedIterable<arraykey, Tv> `](/apis/Classes/HH/ImmSet/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  `` ImmSet ``
* [` ->map<Tu as arraykey>((function(Tv): Tu) $callback): ImmSet<Tu> `](/apis/Classes/HH/ImmSet/map/)\
  Returns an `` ImmSet `` containing the values after an operation has been
  applied to each value in the current ``` ImmSet ```
* [` ->mapWithKey<Tu as arraykey>((function(arraykey, Tv): Tu) $callback): ImmSet<Tu> `](/apis/Classes/HH/ImmSet/mapWithKey/)\
  Returns an `` ImmSet `` containing the values after an operation has been
  applied to each "key" and value in the current ``` ImmSet ```
* [` ->skip(int $n): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/skip/)\
  Returns an `` ImmSet `` containing the values after the ``` n ```-th element of the
  current ```` ImmSet ````
* [` ->skipWhile((function(Tv): bool) $fn): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/skipWhile/)\
  Returns an `` ImmSet `` containing the values of the current ``` ImmSet ``` starting
  after and including the first value that produces ```` true ```` when passed to
  the specified callback
* [` ->slice(int $start, int $len): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/slice/)\
  Returns a subset of the current `` ImmSet `` starting from a given key up to,
  but not including, the element at the provided length from the starting
  key
* [` ->take(int $n): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/take/)\
  Returns an `` ImmSet `` containing the first n values of the current ``` ImmSet ```
* [` ->takeWhile((function(Tv): bool) $callback): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/takeWhile/)\
  Returns an `` ImmSet `` containing the values of the current ``` ImmSet ``` up to
  but not including the first value that produces ```` false ```` when passed to the
  specified callback
* [` ->toDArray(): darray<Tv, Tv> `](/apis/Classes/HH/ImmSet/toDArray/)\
  Returns a darray built from the values from this ImmSet, darray[val1 =>
  val1, val2 => val2, ...]
* [` ->toImmMap(): ImmMap<arraykey, Tv> `](/apis/Classes/HH/ImmSet/toImmMap/)\
  Returns an immutable map ([` ImmMap `](/apis/Classes/HH/ImmMap/)) based on the values of the current
  `` ImmSet ``
* [` ->toImmSet(): ImmSet<Tv> `](/apis/Classes/HH/ImmSet/)\
  Returns an immutable copy (`` ImmSet ``) of the current ``` ImmSet ```
* [` ->toImmVector(): ImmVector<Tv> `](/apis/Classes/HH/ImmSet/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) with the values of the current
  `` ImmSet ``
* [` ->toKeysArray(): varray<Tv> `](/apis/Classes/HH/ImmSet/toKeysArray/)\
  Returns an `` array `` containing the values from the current ``` ImmSet ```
* [` ->toMap(): object `](/apis/Classes/HH/ImmSet/toMap/)\
  Returns a Map built from the keys and values of this ImmSet
* [` ->toSet(): object `](/apis/Classes/HH/ImmSet/toSet/)\
  Returns a Set built from the values of this ImmSet
* [` ->toVArray(): varray<Tv> `](/apis/Classes/HH/ImmSet/toVArray/)
* [` ->toValuesArray(): varray<Tv> `](/apis/Classes/HH/ImmSet/toValuesArray/)\
  Returns an `` array `` containing the values from the current ``` ImmSet ```
* [` ->toVector(): object `](/apis/Classes/HH/ImmSet/toVector/)\
  Returns a Vector built from the values of this ImmSet
* [` ->values(): ImmVector<Tv> `](/apis/Classes/HH/ImmSet/values/)\
  Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current `` ImmSet ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): ImmSet<nothing> `](/apis/Classes/HH/ImmSet/zip/)\
  Throws an exception unless the current `` ImmSet `` or the [` Traversable `](/apis/Interfaces/HH/Traversable/) is
  empty
<!-- HHAPIDOC -->
