
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values of the current [` ImmSet `](/apis/Classes/HH/ImmSet/) that
meet a supplied condition applied to its "keys" and values




``` Hack
public function filterWithKey(
  (function(arraykey, Tv): bool) $callback,
): ImmSet<Tv>;
```




Since [` ImmSet `](/apis/Classes/HH/ImmSet/)s don't have keys, the callback uses the values as the keys
as well.




Only values that meet a certain criteria are affected by a call to
[` filterWithKey() `](/apis/Classes/HH/ImmSet/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Classes/HH/ImmSet/mapWithKey/).




## Parameters




+ ` (function(arraykey, Tv): bool) $callback `




## Returns




* [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values after a user-specified
  condition is applied to the values of the current [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Examples




See [` Set::filterWithKey `](/apis/Classes/HH/Set/filterWithKey) for usage examples.
<!-- HHAPIDOC -->
