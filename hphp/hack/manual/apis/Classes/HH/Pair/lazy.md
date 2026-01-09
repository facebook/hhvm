
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access elements only when needed view of the current
[` Pair `](/docs/apis/Classes/HH/Pair/)




``` Hack
public function lazy(): KeyedIterable<int, mixed>;
```




Normally, memory is allocated for all of the elements of the [` Pair `](/docs/apis/Classes/HH/Pair/).
With a lazy view, memory is allocated for an element only when needed or
used in a calculation like in [` map() `](/docs/apis/Classes/HH/Pair/map/) or [` filter() `](/docs/apis/Classes/HH/Pair/filter/).




That said, [` Pair `](/docs/apis/Classes/HH/Pair/)s only have two elements. So the performance impact on
a [` Pair `](/docs/apis/Classes/HH/Pair/) will be not be noticeable in the real world.




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Returns




* [` KeyedIterable<int, `](/docs/apis/Interfaces/HH/KeyedIterable/)`` mixed> `` - an integer-keyed KeyedIterable representing the lazy view into
  the current [` Pair `](/docs/apis/Classes/HH/Pair/).
<!-- HHAPIDOC -->
