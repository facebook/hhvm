
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access-elements-only-when-needed view of the current
[` ImmVector `](/apis/Classes/HH/ImmVector/)




``` Hack
public function lazy(): KeyedIterable<int, Tv>;
```




Normally, memory is allocated for all of the elements of an [` ImmVector `](/apis/Classes/HH/ImmVector/).
With a lazy view, memory is allocated for an element only when needed or
used in a calculation like in [` map() `](/apis/Classes/HH/ImmVector/map/) or [` filter() `](/apis/Classes/HH/ImmVector/filter/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Returns




* [` KeyedIterable<int, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - An integer-keyed [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) representing the lazy view into
  the current [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Examples




See [` Vector::lazy `](/apis/Classes/HH/Vector/lazy/#examples) for usage examples.
<!-- HHAPIDOC -->
