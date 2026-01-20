
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the values of the current [` ImmMap `](/apis/Classes/HH/ImmMap/) starting
after and including the first value that produces `` true `` when passed to
the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): ImmMap<Tk, Tv>;
```




The returned [` ImmMap `](/apis/Classes/HH/ImmMap/) will always be a proper subset of the current
[` ImmMap `](/apis/Classes/HH/ImmMap/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  [` ImmMap `](/apis/Classes/HH/ImmMap/).




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An [` ImmMap `](/apis/Classes/HH/ImmMap/) that is a proper subset of the current [` ImmMap `](/apis/Classes/HH/ImmMap/)
  starting after the callback returns `` true ``.




## Examples




See [` Map::skipWhile `](/apis/Classes/HH/Map/skipWhile) for usage examples.
<!-- HHAPIDOC -->
