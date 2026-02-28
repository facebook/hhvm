
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values of the current
[` MutableVector `](/apis/Interfaces/MutableVector/) that meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): MutableVector<Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/apis/Interfaces/MutableVector/filter/), while all values are affected by a call to [` map() `](/apis/Interfaces/MutableVector/map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The $fn containing the condition to apply to the
  [` MutableVector `](/apis/Interfaces/MutableVector/) values.




## Returns




- [` MutableVector<Tv> `](/apis/Interfaces/MutableVector/) - a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values after a user-specified
  condition is applied.
<!-- HHAPIDOC -->
