
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values of the current
[` MutableVector `](/apis/Interfaces/MutableVector/) that meet a supplied condition applied to its keys and
values




``` Hack
public function filterWithKey(
  (function(int, Tv): bool) $fn,
): MutableVector<Tv>;
```




Only keys and values that meet a certain criteria are affected by a call to
[` filterWithKey() `](/apis/Interfaces/MutableVector/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Interfaces/MutableVector/mapWithKey/).




## Parameters




+ ` (function(int, Tv): bool) $fn ` - The callback containing the condition to apply to the
  [` MutableVector `](/apis/Interfaces/MutableVector/) keys and values.




## Returns




* [` MutableVector<Tv> `](/apis/Interfaces/MutableVector/) - a [` MutableVector `](/apis/Interfaces/MutableVector/) containing the values after a user-specified
  condition is applied to the keys and values of the current
  [` MutableVector `](/apis/Interfaces/MutableVector/).
<!-- HHAPIDOC -->
