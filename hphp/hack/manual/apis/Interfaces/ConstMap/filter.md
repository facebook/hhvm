
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstMap `](/docs/apis/Interfaces/ConstMap/) containing the values of the current [` ConstMap `](/docs/apis/Interfaces/ConstMap/) that
meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): ConstMap<Tk, Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/docs/apis/Interfaces/ConstMap/filter/), while all values are affected by a call to [` map() `](/docs/apis/Interfaces/ConstMap/map/).




The keys associated with the current [` ConstMap `](/docs/apis/Interfaces/ConstMap/) remain unchanged in the
returned [` ConstMap `](/docs/apis/Interfaces/ConstMap/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The callback containing the condition to apply to the current
  [` ConstMap `](/docs/apis/Interfaces/ConstMap/) values.




## Returns




- [` ConstMap<Tk, `](/docs/apis/Interfaces/ConstMap/)`` Tv> `` - a Map containing the values after a user-specified condition
  is applied.
<!-- HHAPIDOC -->
