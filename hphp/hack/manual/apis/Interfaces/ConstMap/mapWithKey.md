
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstMap `](/apis/Interfaces/ConstMap/) after an operation has been applied to each key and
value in the current [` ConstMap `](/apis/Interfaces/ConstMap/)




``` Hack
public function mapWithKey<Tu>(
  (function(Tk, Tv): Tu) $fn,
): ConstMap<Tk, Tu>;
```




Every key and value in the current [` ConstMap `](/apis/Interfaces/ConstMap/) is affected by a call to
[` mapWithKey() `](/apis/Interfaces/ConstMap/mapWithKey/), unlike [` filterWithKey() `](/apis/Interfaces/ConstMap/filterWithKey/) where only values that meet a
certain criteria are affected.




The keys will remain unchanged from this [` ConstMap `](/apis/Interfaces/ConstMap/) to the returned
[` ConstMap `](/apis/Interfaces/ConstMap/). The keys are only used to help in the mapping operation.




## Parameters




+ ` (function(Tk, Tv): Tu) $fn ` - The callback containing the operation to apply to the current
  [` ConstMap `](/apis/Interfaces/ConstMap/) keys and values.




## Returns




* [` ConstMap<Tk, `](/apis/Interfaces/ConstMap/)`` Tu> `` - a [` ConstMap `](/apis/Interfaces/ConstMap/) containing the values after a user-specified
  operation on the current [` ConstMap `](/apis/Interfaces/ConstMap/)'s keys and values is applied.
<!-- HHAPIDOC -->
