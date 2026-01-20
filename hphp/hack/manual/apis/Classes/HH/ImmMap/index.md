---
title: ImmMap
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` ImmMap ` is an immutable `` Map ``




HHVM provides a native implementation for
this class. The PHP class definition below is not actually used at run time;
it is simply provided for the typechecker and for developer reference.




A ` ImmMap ` cannot be mutated. No elements can be added or removed from it,
nor can elements be overwritten using assignment (i.e. `` $c[$k] = $v `` is
not allowed).




Construct it with a [` Traversable `](/apis/Interfaces/HH/Traversable/):




```
$a = dict['a' => 1, 'b' => 2];
$fm = new ImmMap($a);
```




or use the literal syntax




```
$fm = ImmMap {'a' => 1, 'b' => 2};
```




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Classes](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

final class ImmMap implements \ConstMap<Tk, Tv> {...}
```




### Public Methods




* [` ::fromItems(?Traversable<Pair<Tk, Tv>> $iterable): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/fromItems/)\
  Creates an `` ImmMap `` from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty `` ImmMap ``
  if ``` null ``` is passed
* [` ->__construct(?KeyedTraversable<Tk, Tv> $iterable = NULL): void `](/apis/Classes/HH/ImmMap/__construct/)\
  Creates an `` ImmMap `` from the given [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/), or an empty
  `` ImmMap `` if ``` null ``` is passed
* [` ->__toString(): string `](/apis/Classes/HH/ImmMap/__toString/)\
  Returns the `` string `` version of the current ``` ImmMap ```, which is ```` "ImmMap" ````
* [` ->at(Tk $key): Tv `](/apis/Classes/HH/ImmMap/at/)\
  Returns the value at the specified key in the current `` ImmMap ``
* [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): ImmVector<Tu> `](/apis/Classes/HH/ImmMap/concat/)\
  Returns an ImmVector that is the concatenation of the values of the
  current `` ImmMap `` and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
* [` ->contains(mixed $key): bool `](/apis/Classes/HH/ImmMap/contains/)\
  Determines if the specified key is in the current `` ImmMap ``
* [` ->containsKey(mixed $key): bool `](/apis/Classes/HH/ImmMap/containsKey/)\
  Determines if the specified key is in the current `` ImmMap ``
* [` ->count(): int `](/apis/Classes/HH/ImmMap/count/)\
  Provides the number of elements in the current `` ImmMap ``
* [` ->differenceByKey(KeyedTraversable<mixed, mixed> $traversable): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/differenceByKey/)\
  Returns a new `` ImmMap `` with the keys that are in the current ``` ImmMap ```, but
  not in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)
* [` ->filter((function(Tv): bool) $callback): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/filter/)\
  Returns an `` ImmMap `` containing the values of the current ``` ImmMap ``` that
  meet a supplied condition
* [` ->filterWithKey((function(Tk, Tv): bool) $callback): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/filterWithKey/)\
  Returns an `` ImmMap `` containing the values of the current ``` ImmMap ``` that
  meet a supplied condition applied to its keys and values
* [` ->firstKey(): ?Tk `](/apis/Classes/HH/ImmMap/firstKey/)\
  Returns the first key in the current `` ImmMap ``
* [` ->firstValue(): ?Tv `](/apis/Classes/HH/ImmMap/firstValue/)\
  Returns the first value in the current `` ImmMap ``
* [` ->get(Tk $key): ?Tv `](/apis/Classes/HH/ImmMap/get/)\
  Returns the value at the specified key in the current `` ImmMap ``
* [` ->getIterator(): KeyedIterator<Tk, Tv> `](/apis/Classes/HH/ImmMap/getIterator/)\
  Returns an iterator that points to beginning of the current `` ImmMap ``
* [` ->immutable(): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/immutable/)\
  Returns an immutable copy (`` ImmMap ``) of the current ``` ImmMap ```
* [` ->isEmpty(): bool `](/apis/Classes/HH/ImmMap/isEmpty/)\
  Checks if the current `` ImmMap `` is empty
* [` ->items(): Iterable<Pair<Tk, Tv>> `](/apis/Classes/HH/ImmMap/items/)\
  Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) view of the current `` ImmMap ``
* [` ->keys(): ImmVector<Tk> `](/apis/Classes/HH/ImmMap/keys/)\
  Returns an ImmVector containing, as values, the keys of the current `` ImmMap ``
* [` ->lastKey(): ?Tk `](/apis/Classes/HH/ImmMap/lastKey/)\
  Returns the last key in the current `` ImmMap ``
* [` ->lastValue(): ?Tv `](/apis/Classes/HH/ImmMap/lastValue/)\
  Returns the last value in the current `` ImmMap ``
* [` ->lazy(): KeyedIterable<Tk, Tv> `](/apis/Classes/HH/ImmMap/lazy/)\
  Returns a lazy, access elements only when needed view of the current
  `` ImmMap ``
* [` ->map<Tu>((function(Tv): Tu) $callback): ImmMap<Tk, Tu> `](/apis/Classes/HH/ImmMap/map/)\
  Returns an `` ImmMap `` after an operation has been applied to each value in
  the current ``` ImmMap ```
* [` ->mapWithKey<Tu>((function(Tk, Tv): Tu) $callback): ImmMap<Tk, Tu> `](/apis/Classes/HH/ImmMap/mapWithKey/)\
  Returns an `` ImmMap `` after an operation has been applied to each key and
  value in current ``` ImmMap ```
* [` ->skip(int $n): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/skip/)\
  Returns an `` ImmMap `` containing the values after the ``` n ```-th element of the
  current ```` ImmMap ````
* [` ->skipWhile((function(Tv): bool) $fn): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/skipWhile/)\
  Returns an `` ImmMap `` containing the values of the current ``` ImmMap ``` starting
  after and including the first value that produces ```` true ```` when passed to
  the specified callback
* [` ->slice(int $start, int $len): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/slice/)\
  Returns a subset of the current `` ImmMap `` starting from a given key
  location up to, but not including, the element at the provided length from
  the starting key location
* [` ->take(int $n): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/take/)\
  Returns an `` ImmMap `` containing the first ``` n ``` key/values of the current
  ```` ImmMap ````
* [` ->takeWhile((function(Tv): bool) $callback): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/takeWhile/)\
  Returns an `` ImmMap `` containing the keys and values of the current ``` ImmMap ```
  up to but not including the first value that produces ```` false ```` when passed
  to the specified callback
* [` ->toDArray(): darray<Tk, Tv> `](/apis/Classes/HH/ImmMap/toDArray/)\
  Returns a darray built from the keys and values from this ImmMap
* [` ->toImmMap(): ImmMap<Tk, Tv> `](/apis/Classes/HH/ImmMap/)\
  Returns an immutable copy (`` ImmMap ``) of the current ``` ImmMap ```
* [` ->toImmSet(): ImmSet<Tv> `](/apis/Classes/HH/ImmMap/toImmSet/)\
  Returns an immutable set ([` ImmSet `](/apis/Classes/HH/ImmSet/)) based on the values of the current
  `` ImmMap ``
* [` ->toImmVector(): ImmVector<Tv> `](/apis/Classes/HH/ImmMap/toImmVector/)\
  Returns an immutable vector ([` ImmVector `](/apis/Classes/HH/ImmVector/)) with the values of the current
  `` ImmMap ``
* [` ->toKeysArray(): varray<Tk> `](/apis/Classes/HH/ImmMap/toKeysArray/)\
  Returns an `` array `` whose values are the keys of the current ``` ImmMap ```
* [` ->toMap(): object `](/apis/Classes/HH/ImmMap/toMap/)\
  Returns a Map built from the keys and values of this ImmMap
* [` ->toSet(): object `](/apis/Classes/HH/ImmMap/toSet/)\
  Returns a Set built from the values of this ImmMap
* [` ->toVArray(): varray<Tv> `](/apis/Classes/HH/ImmMap/toVArray/)
* [` ->toValuesArray(): varray<Tv> `](/apis/Classes/HH/ImmMap/toValuesArray/)\
  Returns an `` array `` containing the values from the current ``` ImmMap ```
* [` ->toVector(): object `](/apis/Classes/HH/ImmMap/toVector/)\
  Returns a Vector built from the values of this ImmMap
* [` ->values(): ImmVector<Tv> `](/apis/Classes/HH/ImmMap/values/)\
  Returns an ImmVector containing the values of the current `` ImmMap ``
* [` ->zip<Tu>(Traversable<Tu> $traversable): ImmMap<Tk, Pair<Tv, Tu>> `](/apis/Classes/HH/ImmMap/zip/)\
  Returns an `` ImmMap `` where each value is a [` Pair `](/apis/Classes/HH/Pair/) that combines the value
  of the current `` ImmMap `` and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)
<!-- HHAPIDOC -->
