
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) containing the values of the current [` Iterable `](/apis/Interfaces/HH/Iterable/)
starting after and including the first value that produces `` true `` when
passed to the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): Iterable<Tv>;
```




The returned [` Iterable `](/apis/Interfaces/HH/Iterable/) will always be a proper subset of the current
[` Iterable `](/apis/Interfaces/HH/Iterable/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  returned [` Iterable `](/apis/Interfaces/HH/Iterable/).




## Returns




* [` Iterable<Tv> `](/apis/Interfaces/HH/Iterable/) - An [` Iterable `](/apis/Interfaces/HH/Iterable/) that is a proper subset of the current [` Iterable `](/apis/Interfaces/HH/Iterable/)
  starting after the callback returns `` true ``.
<!-- HHAPIDOC -->
