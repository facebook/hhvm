
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstSet `](/docs/apis/Interfaces/ConstSet/) containing the values of the current [` ConstSet `](/docs/apis/Interfaces/ConstSet/) that
meet a supplied condition applied to each value




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): ConstSet<Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/docs/apis/Interfaces/ConstSet/filter/), while all values are affected by a call to [` map() `](/docs/apis/Interfaces/ConstSet/map/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The callback containing the condition to apply to the
  current [` ConstSet `](/docs/apis/Interfaces/ConstSet/) values.




## Returns




- [` ConstSet<Tv> `](/docs/apis/Interfaces/ConstSet/) - a [` ConstSet `](/docs/apis/Interfaces/ConstSet/) containing the values after a user-specified
  condition is applied.
<!-- HHAPIDOC -->
