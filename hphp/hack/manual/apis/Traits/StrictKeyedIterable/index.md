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




+ [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu> `](/apis/Traits/StrictKeyedIterable/concat/)
+ [` ->filter((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/filter/)
+ [` ->filterWithKey((function(Tk, Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/filterWithKey/)
+ [` ->firstKey(): ?Tk `](/apis/Traits/StrictKeyedIterable/firstKey/)
+ [` ->firstValue(): ?Tv `](/apis/Traits/StrictKeyedIterable/firstValue/)
+ [` ->keys(): Iterable<Tk> `](/apis/Traits/StrictKeyedIterable/keys/)
+ [` ->lastKey(): ?Tk `](/apis/Traits/StrictKeyedIterable/lastKey/)
+ [` ->lastValue(): ?Tv `](/apis/Traits/StrictKeyedIterable/lastValue/)
+ [` ->lazy(): KeyedIterable<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/lazy/)
+ [` ->map<Tu>((function(Tv): Tu) $fn): KeyedIterable<Tk, Tu> `](/apis/Traits/StrictKeyedIterable/map/)
+ [` ->mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): KeyedIterable<Tk, Tu> `](/apis/Traits/StrictKeyedIterable/mapWithKey/)
+ [` ->skip(int $n): KeyedIterable<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/skip/)
+ [` ->skipWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/skipWhile/)
+ [` ->slice(int $start, int $len): KeyedIterable<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/slice/)
+ [` ->take(int $n): KeyedIterable<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/take/)
+ [` ->takeWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/takeWhile/)
+ [` ->toArray() `](/apis/Traits/StrictKeyedIterable/toArray/)
+ [` ->toImmMap(): ImmMap<Tk, Tv> `](/apis/Traits/StrictKeyedIterable/toImmMap/)
+ [` ->toImmSet(): ImmSet<Tv> `](/apis/Traits/StrictKeyedIterable/toImmSet/)
+ [` ->toImmVector(): ImmVector<Tv> `](/apis/Traits/StrictKeyedIterable/toImmVector/)
+ [` ->toKeysArray(): varray<Tk> `](/apis/Traits/StrictKeyedIterable/toKeysArray/)
+ [` ->toMap() `](/apis/Traits/StrictKeyedIterable/toMap/)
+ [` ->toSet() `](/apis/Traits/StrictKeyedIterable/toSet/)
+ [` ->toValuesArray(): varray<Tv> `](/apis/Traits/StrictKeyedIterable/toValuesArray/)
+ [` ->toVector() `](/apis/Traits/StrictKeyedIterable/toVector/)
+ [` ->values(): Iterable<Tv> `](/apis/Traits/StrictKeyedIterable/values/)
+ [` ->zip<Tu>(Traversable<Tu> $traversable): KeyedIterable<Tk, Pair<Tv, Tu>> `](/apis/Traits/StrictKeyedIterable/zip/)







### Public Methods ([` HH\KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/))




* [` ->getIterator(): KeyedIterator<Tk, Tv> `](/apis/Interfaces/HH/KeyedIterable/getIterator/)\
  Returns an iterator that points to beginning of the current
  `` KeyedIterable ``
<!-- HHAPIDOC -->
