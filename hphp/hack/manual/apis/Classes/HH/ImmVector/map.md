
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the results of applying an operation to
each value in the current [` ImmVector `](/apis/Classes/HH/ImmVector/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $callback,
): ImmVector<Tu>;
```




[` map() `](/apis/Classes/HH/ImmVector/map/)'s result contains a value for every value in the current
[` ImmVector `](/apis/Classes/HH/ImmVector/); unlike [` filter() `](/apis/Classes/HH/ImmVector/filter/), where only values that meet a certain
criterion are included in the resulting [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $callback `




## Returns




- [` ImmVector<Tu> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the results of applying a user-specified
  operation to each value of the current [` ImmVector `](/apis/Classes/HH/ImmVector/) in turn.




## Examples




See [` Vector::map `](/apis/Classes/HH/Vector/map/#examples) for usage examples.
<!-- HHAPIDOC -->
