
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access elements only when needed view of the current
[` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function lazy(): KeyedIterable<int, mixed>;
```




Normally, memory is allocated for all of the elements of the [` Pair `](/apis/Classes/HH/Pair/).
With a lazy view, memory is allocated for an element only when needed or
used in a calculation like in [` map() `](/apis/Classes/HH/Pair/map/) or [` filter() `](/apis/Classes/HH/Pair/filter/).




That said, [` Pair `](/apis/Classes/HH/Pair/)s only have two elements. So the performance impact on
a [` Pair `](/apis/Classes/HH/Pair/) will be not be noticeable in the real world.




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Returns




* [` KeyedIterable<int, `](/apis/Interfaces/HH/KeyedIterable/)`` mixed> `` - an integer-keyed KeyedIterable representing the lazy view into
  the current [` Pair `](/apis/Classes/HH/Pair/).
<!-- HHAPIDOC -->
