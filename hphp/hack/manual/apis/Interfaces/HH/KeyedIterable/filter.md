
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) that meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): KeyedIterable<Tk, Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/apis/Interfaces/HH/KeyedIterable/filter/), while all values are affected by a call to [` map() `](/apis/Interfaces/HH/KeyedIterable/map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The callback containing the condition to apply to the
  `` KeyedItearble `` values.




## Returns




- [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the values after a user-specified
  condition is applied.
<!-- HHAPIDOC -->
