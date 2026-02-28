
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstMap `](/apis/Interfaces/ConstMap/) containing the values of the current [` ConstMap `](/apis/Interfaces/ConstMap/)
starting after and including the first value that produces `` true `` when
passed to the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): ConstMap<Tk, Tv>;
```




The returned [` ConstMap `](/apis/Interfaces/ConstMap/) will always be a proper subset of the current
[` ConstMap `](/apis/Interfaces/ConstMap/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  current [` ConstMap `](/apis/Interfaces/ConstMap/).




## Returns




* [` ConstMap<Tk, `](/apis/Interfaces/ConstMap/)`` Tv> `` - A [` ConstMap `](/apis/Interfaces/ConstMap/) that is a proper subset of the current [` ConstMap `](/apis/Interfaces/ConstMap/)
  starting after the callback returns `` true ``.
<!-- HHAPIDOC -->
