
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the current [` ImmVector `](/apis/Classes/HH/ImmVector/)




``` Hack
public function immutable(): ImmVector<Tv>;
```




Unlike [` Vector `](/apis/Classes/HH/Vector/)'s [` toVector() `](/apis/Classes/HH/ImmVector/toVector/) method, this does not actually return a copy
of the current [` ImmVector `](/apis/Classes/HH/ImmVector/). Since [` ImmVector `](/apis/Classes/HH/ImmVector/)s are immutable, there is no
reason to pay the cost of creating a copy of the current [` ImmVector `](/apis/Classes/HH/ImmVector/).




This method is interchangeable with [` toImmVector() `](/apis/Classes/HH/ImmVector/toImmVector/).




This method is NOT interchangeable with [` values() `](/apis/Classes/HH/ImmVector/values/). [` values() `](/apis/Classes/HH/ImmVector/values/) returns a
new [` ImmVector `](/apis/Classes/HH/ImmVector/) that is a copy of the current [` ImmVector `](/apis/Classes/HH/ImmVector/), and thus incurs
both the cost of copying the current [` ImmVector `](/apis/Classes/HH/ImmVector/), and the memory space
consumed by the new [` ImmVector `](/apis/Classes/HH/ImmVector/).  This may be significant, for large
[` ImmVector `](/apis/Classes/HH/ImmVector/)s.




## Returns




+ [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - The current [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Examples




See [` Vector::immutable `](/apis/Classes/HH/Vector/immutable/#examples) for usage examples.
<!-- HHAPIDOC -->
