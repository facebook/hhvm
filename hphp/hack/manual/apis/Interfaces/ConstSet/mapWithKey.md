
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstSet `](/apis/Interfaces/ConstSet/) containing the values after an operation has been
applied to each "key" and value in the current Set




``` Hack
public function mapWithKey<Tu as arraykey>(
  (function(arraykey, Tv): Tu) $fn,
): ConstSet<Tu>;
```




Since sets don't have keys, the callback uses the values as the keys
as well.




Every value in the current [` ConstSet `](/apis/Interfaces/ConstSet/) is affected by a call to
[` mapWithKey() `](/apis/Interfaces/ConstSet/mapWithKey/), unlike [` filterWithKey() `](/apis/Interfaces/ConstSet/filterWithKey/) where only values that meet a
certain criteria are affected.




## Parameters




+ ` (function(arraykey, Tv): Tu) $fn ` - The callback containing the operation to apply to the
  current [` ConstSet `](/apis/Interfaces/ConstSet/) "keys" and values.




## Returns




* [` ConstSet<Tu> `](/apis/Interfaces/ConstSet/) - a [` ConstSet `](/apis/Interfaces/ConstSet/) containing the values after a user-specified
  operation on the current [` ConstSet `](/apis/Interfaces/ConstSet/)'s values is applied.
<!-- HHAPIDOC -->
