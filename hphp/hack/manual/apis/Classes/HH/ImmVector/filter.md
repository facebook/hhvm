
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the values of the current [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) that
meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $callback,
): ImmVector<Tv>;
```




[` filter() `](/docs/apis/Classes/HH/ImmVector/filter/)'s result contains only values that meet the provided criterion;
unlike [` map() `](/docs/apis/Classes/HH/ImmVector/map/), where a value is included for each value in the original
[` ImmVector `](/docs/apis/Classes/HH/ImmVector/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $callback `




## Returns




- [` ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the values after a user-specified
  condition is applied.




## Examples




See [` Vector::filter `](/docs/apis/Classes/HH/Vector/filter/#examples) for usage examples.
<!-- HHAPIDOC -->
