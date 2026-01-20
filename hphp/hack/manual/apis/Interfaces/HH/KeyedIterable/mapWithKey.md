
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values after an operation has
been applied to each key and value in the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)




``` Hack
public function mapWithKey<Tu>(
  (function(Tk, Tv): Tu) $callback,
): KeyedIterable<Tk, Tu>;
```




Every key and value in the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) is affected by a call to
[` mapWithKey() `](/apis/Interfaces/HH/KeyedIterable/mapWithKey/), unlike [` filterWithKey() `](/apis/Interfaces/HH/KeyedIterable/filterWithKey/) where only values that meet a
certain criteria are affected.




## Parameters




+ ` (function(Tk, Tv): Tu) $callback `




## Returns




* [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tu> `` - a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values after a user-specified
  operation on the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)'s keys and values is
  applied.
<!-- HHAPIDOC -->
