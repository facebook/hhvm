
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstSet `](/apis/Interfaces/ConstSet/) containing the values of the current [` ConstSet `](/apis/Interfaces/ConstSet/) up to
but not including the first value that produces `` false `` when passed to the
specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $fn,
): ConstSet<Tv>;
```




The returned [` ConstSet `](/apis/Interfaces/ConstSet/) will always be a proper subset of the current
[` ConstSet `](/apis/Interfaces/ConstSet/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback that is used to determine the stopping condition.




## Returns




* [` ConstSet<Tv> `](/apis/Interfaces/ConstSet/) - A [` ConstSet `](/apis/Interfaces/ConstSet/) that is a proper subset of the current [` ConstSet `](/apis/Interfaces/ConstSet/)
  up until the callback returns `` false ``.
<!-- HHAPIDOC -->
