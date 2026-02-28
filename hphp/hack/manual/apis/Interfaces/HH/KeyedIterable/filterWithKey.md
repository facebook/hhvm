
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) that meet a supplied condition applied to its keys and
values




``` Hack
public function filterWithKey(
  (function(Tk, Tv): bool) $callback,
): KeyedIterable<Tk, Tv>;
```




Only keys and values that meet a certain criteria are affected by a call to
[` filterWithKey() `](/apis/Interfaces/HH/KeyedIterable/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Interfaces/HH/KeyedIterable/mapWithKey/).




## Parameters




+ ` (function(Tk, Tv): bool) $callback `




## Returns




* [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values after a user-specified
  condition is applied to the keys and values of the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).
<!-- HHAPIDOC -->
