
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` Pair `](/apis/Classes/HH/Pair/) up to
but not including the first value that produces `` false `` when passed to the
specified callback




``` Hack
public function takeWhile(
  (function(mixed): bool) $callback,
): ImmVector<mixed>;
```




## Parameters




+ ` (function(mixed): bool) $callback ` - The callback that is used to determine the stopping
  condition.




## Returns




* [` ImmVector<mixed> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) that contains the values of the current [` Pair `](/apis/Classes/HH/Pair/) up
  until the callback returns `` false ``.
<!-- HHAPIDOC -->
