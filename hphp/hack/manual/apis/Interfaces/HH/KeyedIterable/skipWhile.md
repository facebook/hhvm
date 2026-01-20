
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) starting after and including the first value that produces
`` true `` when passed to the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): KeyedIterable<Tk, Tv>;
```




The returned [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) will always be a proper subset of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  returned [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).




## Returns




* [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - A [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) that is a proper subset of the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) starting after the callback returns `` true ``.
<!-- HHAPIDOC -->
