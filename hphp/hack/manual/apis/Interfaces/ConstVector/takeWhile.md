
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the values of the current [` ConstVector `](/apis/Interfaces/ConstVector/)
up to but not including the first value that produces `` false `` when passed
to the specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $fn,
): ConstVector<Tv>;
```




The returned [` ConstVector `](/apis/Interfaces/ConstVector/) will always be a proper subset of the current
[` ConstVector `](/apis/Interfaces/ConstVector/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback that is used to determine the stopping
  condition.




## Returns




* [` ConstVector<Tv> `](/apis/Interfaces/ConstVector/) - A [` ConstVector `](/apis/Interfaces/ConstVector/) that is a proper subset of the current
  [` ConstVector `](/apis/Interfaces/ConstVector/) up until the callback returns `` false ``.
<!-- HHAPIDOC -->
