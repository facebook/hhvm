
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstSet `](/apis/Interfaces/ConstSet/) containing the values after an operation has been
applied to each value in the current [` ConstSet `](/apis/Interfaces/ConstSet/)




``` Hack
public function map<Tu as arraykey>(
  (function(Tv): Tu) $fn,
): ConstSet<Tu>;
```




Every value in the current [` ConstSet `](/apis/Interfaces/ConstSet/) is affected by a call to [` map() `](/apis/Interfaces/ConstSet/map/),
unlike [` filter() `](/apis/Interfaces/ConstSet/filter/) where only values that meet a certain criteria are
affected.




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $fn ` - The callback containing the operation to apply to the
  current [` ConstSet `](/apis/Interfaces/ConstSet/) values.




## Returns




- [` ConstSet<Tu> `](/apis/Interfaces/ConstSet/) - a [` ConstSet `](/apis/Interfaces/ConstSet/) containing the values after a user-specified
  operation is applied.
<!-- HHAPIDOC -->
