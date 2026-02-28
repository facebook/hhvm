
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` Pair `](/apis/Classes/HH/Pair/) that
meet a supplied condition applied to its keys and values




``` Hack
public function filterWithKey(
  (function(int, mixed): bool) $callback,
): ImmVector<mixed>;
```




Only keys and values that meet a certain criteria are affected by a call to
[` filterWithKey() `](/apis/Classes/HH/Pair/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Classes/HH/Pair/mapWithKey/).




## Parameters




+ ` (function(int, mixed): bool) $callback ` - The callback containing the condition to apply to the
  current [` Pair `](/apis/Classes/HH/Pair/) keys and values.




## Returns




* [` ImmVector<mixed> `](/apis/Classes/HH/ImmVector/) - an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after a user-specified
  condition is applied to the keys and values of the current
  [` Pair `](/apis/Classes/HH/Pair/).
<!-- HHAPIDOC -->
