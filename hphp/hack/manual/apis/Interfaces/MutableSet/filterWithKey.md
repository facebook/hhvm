
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values of the current [` MutableSet `](/apis/Interfaces/MutableSet/)
that meet a supplied condition applied to its "keys" and values




``` Hack
public function filterWithKey(
  (function(arraykey, Tv): bool) $fn,
): MutableSet<Tv>;
```




Since sets don't have keys, the callback uses the values as the keys
as well.




Only values that meet a certain criteria are affected by a call to
[` filterWithKey() `](/apis/Interfaces/MutableSet/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Interfaces/MutableSet/mapWithKey/).




## Parameters




+ ` (function(arraykey, Tv): bool) $fn ` - The callback containing the condition to apply to the
  current [` MutableSet `](/apis/Interfaces/MutableSet/) "keys" and values.




## Returns




* [` MutableSet<Tv> `](/apis/Interfaces/MutableSet/) - a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values after a user-specified
  condition is applied to the values of the current [` MutableSet `](/apis/Interfaces/MutableSet/).
<!-- HHAPIDOC -->
