
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) containing the values of the current
[` MutableVector `](/docs/apis/Interfaces/MutableVector/) that meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): MutableVector<Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/docs/apis/Interfaces/MutableVector/filter/), while all values are affected by a call to [` map() `](/docs/apis/Interfaces/MutableVector/map/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The $fn containing the condition to apply to the
  [` MutableVector `](/docs/apis/Interfaces/MutableVector/) values.




## Returns




- [` MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/) - a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) containing the values after a user-specified
  condition is applied.
<!-- HHAPIDOC -->
