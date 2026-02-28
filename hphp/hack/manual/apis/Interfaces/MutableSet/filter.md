
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values of the current [` MutableSet `](/apis/Interfaces/MutableSet/)
that meet a supplied condition applied to each value




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): MutableSet<Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/apis/Interfaces/MutableSet/filter/), while all values are affected by a call to [` map() `](/apis/Interfaces/MutableSet/map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The callback containing the condition to apply to the
  current [` MutableSet `](/apis/Interfaces/MutableSet/) values.




## Returns




- [` MutableSet<Tv> `](/apis/Interfaces/MutableSet/) - a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values after a user-specified
  condition is applied.
<!-- HHAPIDOC -->
