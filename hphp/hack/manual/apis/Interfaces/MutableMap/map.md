
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableMap `](/apis/Interfaces/MutableMap/) after an operation has been applied to each value
in the current [` MutableMap `](/apis/Interfaces/MutableMap/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $fn,
): MutableMap<Tk, Tu>;
```




Every value in the current Map is affected by a call to [` map() `](/apis/Interfaces/MutableMap/map/), unlike
[` filter() `](/apis/Interfaces/MutableMap/filter/) where only values that meet a certain criteria are affected.




The keys will remain unchanged from the current [` MutableMap `](/apis/Interfaces/MutableMap/) to the
returned [` MutableMap `](/apis/Interfaces/MutableMap/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $fn ` - The callback containing the operation to apply to the current
  [` MutableMap `](/apis/Interfaces/MutableMap/) values.




## Returns




- [` MutableMap<Tk, `](/apis/Interfaces/MutableMap/)`` Tu> `` - a [` MutableMap `](/apis/Interfaces/MutableMap/) containing key/value pairs after a user-specified
  operation is applied.
<!-- HHAPIDOC -->
