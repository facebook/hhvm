
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the current ` ImmVector `




``` Hack
public function toImmVector(): ImmVector<Tv>;
```




Unlike ` Vector `'s [` toVector() `](/apis/Classes/HH/ImmVector/toVector/) method, this does not actually return a copy
of the current `` ImmVector ``. Since ``` ImmVector ```s are immutable, there is no
reason to pay the cost of creating a copy of the current ```` ImmVector ````.




This method is interchangeable with [` immutable() `](/apis/Classes/HH/ImmVector/immutable/).




This method is NOT interchangeable with [` values() `](/apis/Classes/HH/ImmVector/values/). [` values() `](/apis/Classes/HH/ImmVector/values/) returns a
new `` ImmVector `` that is a copy of the current ``` ImmVector ```, and thus incurs
both the cost of copying the current ```` ImmVector ````, and the memory space
consumed by the new ````` ImmVector `````.  This may be significant, for large
`````` ImmVector ``````s.




## Returns




+ [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - The current `` ImmVector ``.




## Examples




See [` Vector::toImmVector `](/apis/Classes/HH/Vector) for usage examples.
<!-- HHAPIDOC -->
