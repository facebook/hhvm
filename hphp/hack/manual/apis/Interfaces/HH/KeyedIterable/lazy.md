
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access elements only when needed view of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)




``` Hack
public function lazy(): KeyedIterable<Tk, Tv>;
```




Normally, memory is allocated for all of the elements of the
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/). With a lazy view, memory is allocated for an element only
when needed or used in a calculation like in [` map() `](/apis/Interfaces/HH/KeyedIterable/map/) or [` filter() `](/apis/Interfaces/HH/KeyedIterable/filter/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Returns




* [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) representing the lazy view into the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).
<!-- HHAPIDOC -->
