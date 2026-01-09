---
title: StrictKeyedIterable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
trait StrictKeyedIterable implements HH\KeyedIterable<Tk, Tv> {...}
```




### Public Methods




+ [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu> `](/docs/apis/Traits/StrictKeyedIterable/concat/)
+ [` ->filter((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/filter/)
+ [` ->filterWithKey((function(Tk, Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/filterWithKey/)
+ [` ->firstKey(): ?Tk `](/docs/apis/Traits/StrictKeyedIterable/firstKey/)
+ [` ->firstValue(): ?Tv `](/docs/apis/Traits/StrictKeyedIterable/firstValue/)
+ [` ->keys(): Iterable<Tk> `](/docs/apis/Traits/StrictKeyedIterable/keys/)
+ [` ->lastKey(): ?Tk `](/docs/apis/Traits/StrictKeyedIterable/lastKey/)
+ [` ->lastValue(): ?Tv `](/docs/apis/Traits/StrictKeyedIterable/lastValue/)
+ [` ->lazy(): KeyedIterable<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/lazy/)
+ [` ->map<Tu>((function(Tv): Tu) $fn): KeyedIterable<Tk, Tu> `](/docs/apis/Traits/StrictKeyedIterable/map/)
+ [` ->mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): KeyedIterable<Tk, Tu> `](/docs/apis/Traits/StrictKeyedIterable/mapWithKey/)
+ [` ->skip(int $n): KeyedIterable<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/skip/)
+ [` ->skipWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/skipWhile/)
+ [` ->slice(int $start, int $len): KeyedIterable<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/slice/)
+ [` ->take(int $n): KeyedIterable<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/take/)
+ [` ->takeWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/takeWhile/)
+ [` ->toArray() `](/docs/apis/Traits/StrictKeyedIterable/toArray/)
+ [` ->toImmMap(): ImmMap<Tk, Tv> `](/docs/apis/Traits/StrictKeyedIterable/toImmMap/)
+ [` ->toImmSet(): ImmSet<Tv> `](/docs/apis/Traits/StrictKeyedIterable/toImmSet/)
+ [` ->toImmVector(): ImmVector<Tv> `](/docs/apis/Traits/StrictKeyedIterable/toImmVector/)
+ [` ->toKeysArray(): varray<Tk> `](/docs/apis/Traits/StrictKeyedIterable/toKeysArray/)
+ [` ->toMap() `](/docs/apis/Traits/StrictKeyedIterable/toMap/)
+ [` ->toSet() `](/docs/apis/Traits/StrictKeyedIterable/toSet/)
+ [` ->toValuesArray(): varray<Tv> `](/docs/apis/Traits/StrictKeyedIterable/toValuesArray/)
+ [` ->toVector() `](/docs/apis/Traits/StrictKeyedIterable/toVector/)
+ [` ->values(): Iterable<Tv> `](/docs/apis/Traits/StrictKeyedIterable/values/)
+ [` ->zip<Tu>(Traversable<Tu> $traversable): KeyedIterable<Tk, Pair<Tv, Tu>> `](/docs/apis/Traits/StrictKeyedIterable/zip/)







### Public Methods ([` HH\KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/))




* [` ->getIterator(): KeyedIterator<Tk, Tv> `](/docs/apis/Interfaces/HH/KeyedIterable/getIterator/)\
  Returns an iterator that points to beginning of the current
  `` KeyedIterable ``
<!-- HHAPIDOC -->
