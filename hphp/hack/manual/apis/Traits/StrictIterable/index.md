---
title: StrictIterable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This file provides type information for some of PHP's predefined interfaces




YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!




## Interface Synopsis




``` Hack
trait StrictIterable implements HH\Iterable<Tv> {...}
```




### Public Methods




+ [` ->concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu> `](/apis/Traits/StrictIterable/concat/)
+ [` ->filter((function(Tv): bool) $fn): Iterable<Tv> `](/apis/Traits/StrictIterable/filter/)
+ [` ->firstValue(): ?Tv `](/apis/Traits/StrictIterable/firstValue/)
+ [` ->lastValue(): ?Tv `](/apis/Traits/StrictIterable/lastValue/)
+ [` ->lazy(): Iterable<Tv> `](/apis/Traits/StrictIterable/lazy/)
+ [` ->map<Tu>((function(Tv): Tu) $fn): Iterable<Tu> `](/apis/Traits/StrictIterable/map/)
+ [` ->skip(int $n): Iterable<Tv> `](/apis/Traits/StrictIterable/skip/)
+ [` ->skipWhile((function(Tv): bool) $fn): Iterable<Tv> `](/apis/Traits/StrictIterable/skipWhile/)
+ [` ->slice(int $start, int $len): Iterable<Tv> `](/apis/Traits/StrictIterable/slice/)
+ [` ->take(int $n): Iterable<Tv> `](/apis/Traits/StrictIterable/take/)
+ [` ->takeWhile((function(Tv): bool) $fn): Iterable<Tv> `](/apis/Traits/StrictIterable/takeWhile/)
+ [` ->toArray() `](/apis/Traits/StrictIterable/toArray/)
+ [` ->toImmSet(): ImmSet<Tv> `](/apis/Traits/StrictIterable/toImmSet/)
+ [` ->toImmVector(): ImmVector<Tv> `](/apis/Traits/StrictIterable/toImmVector/)
+ [` ->toSet() `](/apis/Traits/StrictIterable/toSet/)
+ [` ->toValuesArray(): varray<Tv> `](/apis/Traits/StrictIterable/toValuesArray/)
+ [` ->toVector() `](/apis/Traits/StrictIterable/toVector/)
+ [` ->values(): Iterable<Tv> `](/apis/Traits/StrictIterable/values/)
+ [` ->zip<Tu>(Traversable<Tu> $traversable): Iterable<Pair<Tv, Tu>> `](/apis/Traits/StrictIterable/zip/)







### Public Methods ([` HH\Iterable `](/apis/Interfaces/HH/Iterable/))




* [` ->getIterator(): Iterator<Tv> `](/apis/Interfaces/HH/Iterable/getIterator/)\
  Returns an iterator that points to beginning of the current `` Iterable ``
<!-- HHAPIDOC -->
