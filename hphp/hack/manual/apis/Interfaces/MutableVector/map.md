
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values after an operation has been
applied to each value in the current [` MutableVector `](/apis/Interfaces/MutableVector/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $fn,
): MutableVector<Tu>;
```




Every value in the current [` MutableVector `](/apis/Interfaces/MutableVector/) is affected by a call to
[` map() `](/apis/Interfaces/MutableVector/map/), unlike [` filter() `](/apis/Interfaces/MutableVector/filter/) where only values that meet a certain criteria
are affected.




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $fn ` - The callback containing the operation to apply to the
  [` MutableVector `](/apis/Interfaces/MutableVector/) values.




## Returns




- [` MutableVector<Tu> `](/apis/Interfaces/MutableVector/) - a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values after a user-specified
  operation is applied.
<!-- HHAPIDOC -->
