
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values after an operation has been
applied to each "key" and value in the current Set




``` Hack
public function mapWithKey<Tu as arraykey>(
  (function(arraykey, Tv): Tu) $fn,
): MutableSet<Tu>;
```




Since sets don't have keys, the callback uses the values as the keys
as well.




Every value in the current [` MutableSet `](/apis/Interfaces/MutableSet/) is affected by a call to
[` mapWithKey() `](/apis/Interfaces/MutableSet/mapWithKey/), unlike [` filterWithKey() `](/apis/Interfaces/MutableSet/filterWithKey/) where only values that meet a
certain criteria are affected.




## Parameters




+ ` (function(arraykey, Tv): Tu) $fn ` - The callback containing the operation to apply to the
  current [` MutableSet `](/apis/Interfaces/MutableSet/) "keys" and values.




## Returns




* [` MutableSet<Tu> `](/apis/Interfaces/MutableSet/) - a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values after a user-specified
  operation on the current [` MutableSet `](/apis/Interfaces/MutableSet/)'s values is applied.
<!-- HHAPIDOC -->
