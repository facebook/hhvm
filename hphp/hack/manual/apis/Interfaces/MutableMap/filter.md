
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableMap `](/docs/apis/Interfaces/MutableMap/) containing the values of the current [` MutableMap `](/docs/apis/Interfaces/MutableMap/)
that meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): MutableMap<Tk, Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/docs/apis/Interfaces/MutableMap/filter/), while all values are affected by a call to [` map() `](/docs/apis/Interfaces/MutableMap/map/).




The keys associated with the current [` MutableMap `](/docs/apis/Interfaces/MutableMap/) remain unchanged in the
returned [` MutableMap `](/docs/apis/Interfaces/MutableMap/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The callback containing the condition to apply to the current
  [` MutableMap `](/docs/apis/Interfaces/MutableMap/) values.




## Returns




- [` MutableMap<Tk, `](/docs/apis/Interfaces/MutableMap/)`` Tv> `` - a Map containing the values after a user-specified condition
  is applied.
<!-- HHAPIDOC -->
