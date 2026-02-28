
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableMap `](/apis/Interfaces/MutableMap/) containing the values of the current [` MutableMap `](/apis/Interfaces/MutableMap/)
starting after and including the first value that produces `` true `` when
passed to the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): MutableMap<Tk, Tv>;
```




The returned [` MutableMap `](/apis/Interfaces/MutableMap/) will always be a proper subset of the current
[` MutableMap `](/apis/Interfaces/MutableMap/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  current [` MutableMap `](/apis/Interfaces/MutableMap/).




## Returns




* [` MutableMap<Tk, `](/apis/Interfaces/MutableMap/)`` Tv> `` - A [` MutableMap `](/apis/Interfaces/MutableMap/) that is a proper subset of the current
  [` MutableMap `](/apis/Interfaces/MutableMap/) starting after the callback returns `` true ``.
<!-- HHAPIDOC -->
