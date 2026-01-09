
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the results of applying an operation to
each value in the current [` ImmVector `](/docs/apis/Classes/HH/ImmVector/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $callback,
): ImmVector<Tu>;
```




[` map() `](/docs/apis/Classes/HH/ImmVector/map/)'s result contains a value for every value in the current
[` ImmVector `](/docs/apis/Classes/HH/ImmVector/); unlike [` filter() `](/docs/apis/Classes/HH/ImmVector/filter/), where only values that meet a certain
criterion are included in the resulting [` ImmVector `](/docs/apis/Classes/HH/ImmVector/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $callback `




## Returns




- [` ImmVector<Tu> `](/docs/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the results of applying a user-specified
  operation to each value of the current [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) in turn.




## Examples




See [` Vector::map `](/docs/apis/Classes/HH/Vector/map/#examples) for usage examples.
<!-- HHAPIDOC -->
