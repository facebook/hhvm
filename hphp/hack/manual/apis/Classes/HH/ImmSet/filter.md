
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values of the current [` ImmSet `](/apis/Classes/HH/ImmSet/) that
meet a supplied condition applied to each value




``` Hack
public function filter(
  (function(Tv): bool) $callback,
): ImmSet<Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/apis/Classes/HH/ImmSet/filter/), while all values are affected by a call to [` map() `](/apis/Classes/HH/ImmSet/map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $callback `




## Returns




- [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values after a user-specified
  condition is applied.




## Examples




See [` Set::filter `](/apis/Classes/HH/Set/filter/#examples) for usage examples.
<!-- HHAPIDOC -->
