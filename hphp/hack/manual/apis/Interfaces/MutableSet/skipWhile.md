
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values of the current [` MutableSet `](/apis/Interfaces/MutableSet/)
starting after and including the first value that produces `` true `` when
passed to the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): MutableSet<Tv>;
```




The returned [` MutableSet `](/apis/Interfaces/MutableSet/) will always be a proper subset of the current
[` MutableSet `](/apis/Interfaces/MutableSet/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  [` MutableSet `](/apis/Interfaces/MutableSet/).




## Returns




* [` MutableSet<Tv> `](/apis/Interfaces/MutableSet/) - A [` MutableSet `](/apis/Interfaces/MutableSet/) that is a proper subset of the current
  [` MutableSet `](/apis/Interfaces/MutableSet/) starting after the callback returns `` true ``.
<!-- HHAPIDOC -->
