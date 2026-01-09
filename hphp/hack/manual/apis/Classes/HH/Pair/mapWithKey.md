
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the values after an operation has been
applied to each key and value in the current [` Pair `](/docs/apis/Classes/HH/Pair/)




``` Hack
public function mapWithKey<Tu>(
  (function(int, mixed): Tu) $callback,
): ImmVector<Tu>;
```




Every key and value in the current [` Pair `](/docs/apis/Classes/HH/Pair/) is affected by a call to
[` mapWithKey() `](/docs/apis/Classes/HH/Pair/mapWithKey/), unlike [` filterWithKey() `](/docs/apis/Classes/HH/Pair/filterWithKey/) where only values that meet a
certain criteria are affected.




## Parameters




+ ` (function(int, mixed): Tu) $callback ` - The $allback containing the operation to apply to the
  current [` Pair `](/docs/apis/Classes/HH/Pair/) keys and values.




## Returns




* [` ImmVector<Tu> `](/docs/apis/Classes/HH/ImmVector/) - an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the values after a user-specified
  operation on the current [` Pair `](/docs/apis/Classes/HH/Pair/)'s keys and values is applied.
<!-- HHAPIDOC -->
