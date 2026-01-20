
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable integer-keyed Map (` ImmMap `) based on the elements of
the current [` ImmVector `](/apis/Classes/HH/ImmVector/)




``` Hack
public function toImmMap(): ImmMap<int, Tv>;
```




The keys are ` 0... count() - 1 `.




## Returns




+ [` ImmMap<int, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An integer-keyed ``` ImmMap ``` with the values of the current
  [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Examples




See [` Vector::toImmMap `](/apis/Classes/HH/Vector/toImmMap/#examples) for usage examples.
<!-- HHAPIDOC -->
