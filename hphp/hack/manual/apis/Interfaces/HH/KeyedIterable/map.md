
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values after an operation has been
applied to each value in the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $fn,
): KeyedIterable<Tk, Tu>;
```




Every value in the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) is affected by a call to
[` map() `](/apis/Interfaces/HH/KeyedIterable/map/), unlike [` filter() `](/apis/Interfaces/HH/KeyedIterable/filter/) where only values that meet a certain criteria
are affected.




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $fn ` - The callback containing the operation to apply to the
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) values.




## Returns




- [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tu> `` - a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values after a user-specified
  operation is applied.
<!-- HHAPIDOC -->
