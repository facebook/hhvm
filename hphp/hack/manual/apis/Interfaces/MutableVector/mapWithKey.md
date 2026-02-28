
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values after an operation has been
applied to each key and value in the current [` MutableVector `](/apis/Interfaces/MutableVector/)




``` Hack
public function mapWithKey<Tu>(
  (function(int, Tv): Tu) $fn,
): MutableVector<Tu>;
```




Every key and value in the current [` MutableVector `](/apis/Interfaces/MutableVector/) is affected by a call to
[` mapWithKey() `](/apis/Interfaces/MutableVector/mapWithKey/), unlike [` filterWithKey() `](/apis/Interfaces/MutableVector/filterWithKey/) where only values that meet a
certain criteria are affected.




## Parameters




+ ` (function(int, Tv): Tu) $fn ` - The callback containing the operation to apply to the
  [` MutableVector `](/apis/Interfaces/MutableVector/) keys and values.




## Returns




* [` MutableVector<Tu> `](/apis/Interfaces/MutableVector/) - a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values after a user-specified
  operation on the current Vector's keys and values is applied.
<!-- HHAPIDOC -->
