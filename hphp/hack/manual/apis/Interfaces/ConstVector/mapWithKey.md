
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the values after an operation has been
applied to each key and value in the current [` ConstVector `](/apis/Interfaces/ConstVector/)




``` Hack
public function mapWithKey<Tu>(
  (function(int, Tv): Tu) $fn,
): ConstVector<Tu>;
```




Every key and value in the current [` ConstVector `](/apis/Interfaces/ConstVector/) is affected by a call to
[` mapWithKey() `](/apis/Interfaces/ConstVector/mapWithKey/), unlike [` filterWithKey() `](/apis/Interfaces/ConstVector/filterWithKey/) where only values that meet a
certain criteria are affected.




## Parameters




+ ` (function(int, Tv): Tu) $fn ` - The callback containing the operation to apply to the
  [` ConstVector `](/apis/Interfaces/ConstVector/) keys and values.




## Returns




* [` ConstVector<Tu> `](/apis/Interfaces/ConstVector/) - a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the values after a user-specified
  operation on the current Vector's keys and values is applied.
<!-- HHAPIDOC -->
