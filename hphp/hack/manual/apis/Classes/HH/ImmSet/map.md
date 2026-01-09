
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) containing the values after an operation has been
applied to each value in the current [` ImmSet `](/docs/apis/Classes/HH/ImmSet/)




``` Hack
public function map<Tu as arraykey>(
  (function(Tv): Tu) $callback,
): ImmSet<Tu>;
```




Every value in the current [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) is affected by a call to [` map() `](/docs/apis/Classes/HH/ImmSet/map/),
unlike [` filter() `](/docs/apis/Classes/HH/ImmSet/filter/) where only values that meet a certain criteria are
affected.




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $callback `




## Returns




- [` ImmSet<Tu> `](/docs/apis/Classes/HH/ImmSet/) - a [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) containing the values after a user-specified operation
  is applied.




## Examples




See [` Set::map `](/docs/apis/Classes/HH/Set/map/#examples) for usage examples.
<!-- HHAPIDOC -->
