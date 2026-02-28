
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` ImmVector `](/apis/Classes/HH/ImmVector/)
that meet a supplied condition applied to its keys and values




``` Hack
public function filterWithKey(
  (function(int, Tv): bool) $callback,
): ImmVector<Tv>;
```




[` filterWithKey() `](/apis/Classes/HH/ImmVector/filterWithKey/)'s result contains only values whose key/value pairs
satisfy the provided criterion; unlike [` mapWithKey() `](/apis/Classes/HH/ImmVector/mapWithKey/), which contains
results derived from every key/value pair in the original [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Parameters




+ ` (function(int, Tv): bool) $callback `




## Returns




* [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` ImmVector `](/apis/Classes/HH/ImmVector/)
  for which a user-specified test condition returns true when
  applied to the corresponding key/value pairs.




## Examples




See [` Vector::filterWithKey `](/apis/Classes/HH/Vector/filterWithKey/#examples) for usage examples.
<!-- HHAPIDOC -->
