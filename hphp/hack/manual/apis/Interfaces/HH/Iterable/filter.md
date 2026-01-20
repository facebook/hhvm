
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) containing the values of the current [` Iterable `](/apis/Interfaces/HH/Iterable/) that
meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $fn,
): Iterable<Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/apis/Interfaces/HH/Iterable/filter/), while all values are affected by a call to [` map() `](/apis/Interfaces/HH/Iterable/map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $fn ` - The callback containing the condition to apply to the
  `` Itearble `` values.




## Returns




- [` Iterable<Tv> `](/apis/Interfaces/HH/Iterable/) - an [` Iterable `](/apis/Interfaces/HH/Iterable/) containing the values after a user-specified
  condition is applied.
<!-- HHAPIDOC -->
