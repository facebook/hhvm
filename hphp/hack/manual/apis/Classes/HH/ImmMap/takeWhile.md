
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the keys and values of the current [` ImmMap `](/apis/Classes/HH/ImmMap/)
up to but not including the first value that produces `` false `` when passed
to the specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $callback,
): ImmMap<Tk, Tv>;
```




The returned [` ImmMap `](/apis/Classes/HH/ImmMap/) will always be a proper subset of the current
[` ImmMap `](/apis/Classes/HH/ImmMap/).




## Parameters




+ ` (function(Tv): bool) $callback `




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An [` ImmMap `](/apis/Classes/HH/ImmMap/) that is a proper subset of the current [` ImmMap `](/apis/Classes/HH/ImmMap/) up
  until when the callback returns `` false ``.




## Examples




See [` Map::takeWhile `](/apis/Classes/HH/Map/takeWhile) for usage examples.
<!-- HHAPIDOC -->
