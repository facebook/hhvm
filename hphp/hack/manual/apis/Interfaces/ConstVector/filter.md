
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the values of the current [` ConstVector `](/docs/apis/Interfaces/ConstVector/)
that meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): ConstVector<Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/docs/apis/Interfaces/ConstVector/filter/), while all values are affected by a call to [` map() `](/docs/apis/Interfaces/ConstVector/map/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The $fn containing the condition to apply to the
  [` ConstVector `](/docs/apis/Interfaces/ConstVector/) values.




## Returns




- [` ConstVector<Tv> `](/docs/apis/Interfaces/ConstVector/) - a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the values after a user-specified
  condition is applied.
<!-- HHAPIDOC -->
