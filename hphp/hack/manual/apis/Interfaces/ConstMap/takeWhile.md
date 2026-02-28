
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstMap `](/apis/Interfaces/ConstMap/) containing the keys and values of the current
[` ConstMap `](/apis/Interfaces/ConstMap/) up to but not including the first value that produces `` false ``
when passed to the specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $fn,
): ConstMap<Tk, Tv>;
```




The returned [` ConstMap `](/apis/Interfaces/ConstMap/) will always be a proper subset of the current
[` ConstMap `](/apis/Interfaces/ConstMap/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback that is used to determine the stopping condition.




## Returns




* [` ConstMap<Tk, `](/apis/Interfaces/ConstMap/)`` Tv> `` - A [` ConstMap `](/apis/Interfaces/ConstMap/) that is a proper subset of the current [` ConstMap `](/apis/Interfaces/ConstMap/)
  up until the callback returns `` false ``.
<!-- HHAPIDOC -->
