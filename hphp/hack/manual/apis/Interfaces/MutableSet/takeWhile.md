
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values of the current [` MutableSet `](/apis/Interfaces/MutableSet/)
up to but not including the first value that produces `` false `` when passed
to the specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $fn,
): MutableSet<Tv>;
```




The returned [` MutableSet `](/apis/Interfaces/MutableSet/) will always be a proper subset of the current
[` MutableSet `](/apis/Interfaces/MutableSet/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback that is used to determine the stopping condition.




## Returns




* [` MutableSet<Tv> `](/apis/Interfaces/MutableSet/) - A [` MutableSet `](/apis/Interfaces/MutableSet/) that is a proper subset of the current
  [` MutableSet `](/apis/Interfaces/MutableSet/) up until the callback returns `` false ``.
<!-- HHAPIDOC -->
