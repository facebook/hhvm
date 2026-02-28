
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values after an operation has been
applied to each "key" and value in the current [` ImmSet `](/apis/Classes/HH/ImmSet/)




``` Hack
public function mapWithKey<Tu as arraykey>(
  (function(arraykey, Tv): Tu) $callback,
): ImmSet<Tu>;
```




Since [` ImmSet `](/apis/Classes/HH/ImmSet/)s don't have keys, the callback uses the values as the keys
as well.




Every value in the current [` ImmSet `](/apis/Classes/HH/ImmSet/) is affected by a call to
[` mapWithKey() `](/apis/Classes/HH/ImmSet/mapWithKey/), unlike [` filterWithKey() `](/apis/Classes/HH/ImmSet/filterWithKey/) where only values that meet a
certain criteria are affected.




## Parameters




+ ` (function(arraykey, Tv): Tu) $callback `




## Returns




* [` ImmSet<Tu> `](/apis/Classes/HH/ImmSet/) - an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values after a user-specified
  operation on the current [` ImmSet `](/apis/Classes/HH/ImmSet/)'s values is applied.




## Examples




See [` Set::mapWithKey `](/apis/Classes/HH/Set/mapWithKey) for usage examples.
<!-- HHAPIDOC -->
