
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstMap `](/apis/Interfaces/ConstMap/) after an operation has been applied to each value in
the current [` ConstMap `](/apis/Interfaces/ConstMap/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $fn,
): ConstMap<Tk, Tu>;
```




Every value in the current Map is affected by a call to [` map() `](/apis/Interfaces/ConstMap/map/), unlike
[` filter() `](/apis/Interfaces/ConstMap/filter/) where only values that meet a certain criteria are affected.




The keys will remain unchanged from the current [` ConstMap `](/apis/Interfaces/ConstMap/) to the returned
[` ConstMap `](/apis/Interfaces/ConstMap/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $fn ` - The callback containing the operation to apply to the current
  [` ConstMap `](/apis/Interfaces/ConstMap/) values.




## Returns




- [` ConstMap<Tk, `](/apis/Interfaces/ConstMap/)`` Tu> `` - a [` ConstMap `](/apis/Interfaces/ConstMap/) containing key/value pairs after a user-specified
  operation is applied.
<!-- HHAPIDOC -->
