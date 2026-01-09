
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access elements only when needed view of the current
[` ImmSet `](/docs/apis/Classes/HH/ImmSet/)




``` Hack
public function lazy(): KeyedIterable<arraykey, Tv>;
```




Normally, memory is allocated for all of the elements of an [` ImmSet `](/docs/apis/Classes/HH/ImmSet/). With
a lazy view, memory is allocated for an element only when needed or used
in a calculation like in [` map() `](/docs/apis/Classes/HH/ImmSet/map/) or [` filter() `](/docs/apis/Classes/HH/ImmSet/filter/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Returns




* [` KeyedIterable<arraykey, `](/docs/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - an [` KeyedIterable `](/docs/apis/Interfaces/HH/KeyedIterable/) representing the lazy view into the current
  [` ImmSet `](/docs/apis/Classes/HH/ImmSet/), where the keys are the same as the values.




## Examples




See [` Set::lazy `](/docs/apis/Classes/HH/Set/lazy/#examples) for usage examples.
<!-- HHAPIDOC -->
