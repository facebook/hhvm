
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access elements only when needed view of the current
[` Iterable `](/apis/Interfaces/HH/Iterable/)




``` Hack
public function lazy(): Iterable<Tv>;
```




Normally, memory is allocated for all of the elements of the [` Iterable `](/apis/Interfaces/HH/Iterable/).
With a lazy view, memory is allocated for an element only when needed or
used in a calculation like in [` map() `](/apis/Interfaces/HH/Iterable/map/) or [` filter() `](/apis/Interfaces/HH/Iterable/filter/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Returns




* [` Iterable<Tv> `](/apis/Interfaces/HH/Iterable/) - an [` Iterable `](/apis/Interfaces/HH/Iterable/) representing the lazy view into the current
  [` Iterable `](/apis/Interfaces/HH/Iterable/).
<!-- HHAPIDOC -->
