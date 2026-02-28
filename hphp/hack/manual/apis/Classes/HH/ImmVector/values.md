
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` ImmVector `](/apis/Classes/HH/ImmVector/);
that is, a copy of the current [` ImmVector `](/apis/Classes/HH/ImmVector/)




``` Hack
public function values(): ImmVector<Tv>;
```




This method is NOT interchangeable with [` toImmVector() `](/apis/Classes/HH/ImmVector/toImmVector/) and [` immutable() `](/apis/Classes/HH/ImmVector/immutable/).
[` toImmVector() `](/apis/Classes/HH/ImmVector/toImmVector/) and [` immutable() `](/apis/Classes/HH/ImmVector/immutable/) return the current [` ImmVector `](/apis/Classes/HH/ImmVector/), and do
not incur the cost of copying the current [` ImmVector `](/apis/Classes/HH/ImmVector/), or the memory space
consumed by the new [` ImmVector `](/apis/Classes/HH/ImmVector/).  This may be significant, for large
[` ImmVector `](/apis/Classes/HH/ImmVector/)s.




## Returns




+ [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - A new [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current
  [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Examples




See [` Vector::values `](/apis/Classes/HH/Vector/values/#examples) for usage examples.
<!-- HHAPIDOC -->
