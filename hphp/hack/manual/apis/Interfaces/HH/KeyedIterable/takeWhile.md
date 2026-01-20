
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) up to but not including the first value that produces
`` false `` when passed to the specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $fn,
): KeyedIterable<Tk, Tv>;
```




The returned [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) will always be a proper subset of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback that is used to determine the stopping
  condition.




## Returns




* [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - A [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) that is a proper subset of the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) up until the callback returns `` false ``.
<!-- HHAPIDOC -->
