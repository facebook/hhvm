
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an immutable map (` ImmMap `) based on the values of the current
[` ImmSet `](/apis/Classes/HH/ImmSet/)




``` Hack
public function toImmMap(): ImmMap<arraykey, Tv>;
```




Each key of the ` Map ` will be the same as its value.




## Returns




+ [` ImmMap<arraykey, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - an ``` ImmMap ``` that that contains the values of the current
  [` ImmSet `](/apis/Classes/HH/ImmSet/), with each key of the `` ImmMap `` being the same as its
  value.




## Examples




See [` Set::toImmMap `](/apis/Classes/HH/Set/toImmMap/#examples) for usage examples.
<!-- HHAPIDOC -->
