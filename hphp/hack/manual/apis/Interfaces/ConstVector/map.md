
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the values after an operation has been
applied to each value in the current [` ConstVector `](/docs/apis/Interfaces/ConstVector/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $fn,
): ConstVector<Tu>;
```




Every value in the current [` ConstVector `](/docs/apis/Interfaces/ConstVector/) is affected by a call to [` map() `](/docs/apis/Interfaces/ConstVector/map/),
unlike [` filter() `](/docs/apis/Interfaces/ConstVector/filter/) where only values that meet a certain criteria are
affected.




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $fn ` - The callback containing the operation to apply to the
  [` ConstVector `](/docs/apis/Interfaces/ConstVector/) values.




## Returns




- [` ConstVector<Tu> `](/docs/apis/Interfaces/ConstVector/) - a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the values after a user-specified
  operation is applied.
<!-- HHAPIDOC -->
