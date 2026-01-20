
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values of the current
[` MutableVector `](/apis/Interfaces/MutableVector/) starting after and including the first value that produces
`` true `` when passed to the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): MutableVector<Tv>;
```




The returned [` MutableVector `](/apis/Interfaces/MutableVector/) will always be a proper subset of the current
[` MutableVector `](/apis/Interfaces/MutableVector/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  returned [` MutableVector `](/apis/Interfaces/MutableVector/).




## Returns




* [` MutableVector<Tv> `](/apis/Interfaces/MutableVector/) - A [` MutableVector `](/apis/Interfaces/MutableVector/) that is a proper subset of the current
  [` MutableVector `](/apis/Interfaces/MutableVector/) starting after the callback returns `` true ``.
<!-- HHAPIDOC -->
