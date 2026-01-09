
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/docs/apis/Interfaces/MutableSet/) containing the values after an operation has been
applied to each value in the current [` MutableSet `](/docs/apis/Interfaces/MutableSet/)




``` Hack
public function map<Tu as arraykey>(
  (function(Tv): Tu) $fn,
): MutableSet<Tu>;
```




Every value in the current [` MutableSet `](/docs/apis/Interfaces/MutableSet/) is affected by a call to [` map() `](/docs/apis/Interfaces/MutableSet/map/),
unlike [` filter() `](/docs/apis/Interfaces/MutableSet/filter/) where only values that meet a certain criteria are
affected.




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $fn ` - The callback containing the operation to apply to the
  current [` MutableSet `](/docs/apis/Interfaces/MutableSet/) values.




## Returns




- [` MutableSet<Tu> `](/docs/apis/Interfaces/MutableSet/) - a [` MutableSet `](/docs/apis/Interfaces/MutableSet/) containing the values after a user-specified
  operation is applied.
<!-- HHAPIDOC -->
