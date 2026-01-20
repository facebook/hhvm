
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableMap `](/apis/Interfaces/MutableMap/) after an operation has been applied to each key and
value in the current [` MutableMap `](/apis/Interfaces/MutableMap/)




``` Hack
public function mapWithKey<Tu>(
  (function(Tk, Tv): Tu) $fn,
): MutableMap<Tk, Tu>;
```




Every key and value in the current [` MutableMap `](/apis/Interfaces/MutableMap/) is affected by a call to
[` mapWithKey() `](/apis/Interfaces/MutableMap/mapWithKey/), unlike [` filterWithKey() `](/apis/Interfaces/MutableMap/filterWithKey/) where only values that meet a
certain criteria are affected.




The keys will remain unchanged from this [` MutableMap `](/apis/Interfaces/MutableMap/) to the returned
[` MutableMap `](/apis/Interfaces/MutableMap/). The keys are only used to help in the mapping operation.




## Parameters




+ ` (function(Tk, Tv): Tu) $fn ` - The callback containing the operation to apply to the current
  [` MutableMap `](/apis/Interfaces/MutableMap/) keys and values.




## Returns




* [` MutableMap<Tk, `](/apis/Interfaces/MutableMap/)`` Tu> `` - a [` MutableMap `](/apis/Interfaces/MutableMap/) containing the values after a user-specified
  operation on the current [` MutableMap `](/apis/Interfaces/MutableMap/)'s keys and values is
  applied.
<!-- HHAPIDOC -->
