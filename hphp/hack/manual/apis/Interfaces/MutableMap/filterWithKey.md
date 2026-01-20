
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableMap `](/apis/Interfaces/MutableMap/) containing the values of the current [` MutableMap `](/apis/Interfaces/MutableMap/)
that meet a supplied condition applied to its keys and values




``` Hack
public function filterWithKey(
  (function(Tk, Tv): bool) $fn,
): MutableMap<Tk, Tv>;
```




Only keys and values that meet a certain criteria are affected by a call
to [` filterWithKey() `](/apis/Interfaces/MutableMap/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Interfaces/MutableMap/mapWithKey/).




The keys associated with the current [` MutableMap `](/apis/Interfaces/MutableMap/) remain unchanged in the
returned [` MutableMap `](/apis/Interfaces/MutableMap/); the keys will be used in the filtering process only.




## Parameters




+ ` (function(Tk, Tv): bool) $fn ` - The callback containing the condition to apply to the current
  [` MutableMap `](/apis/Interfaces/MutableMap/) keys and values.




## Returns




* [` MutableMap<Tk, `](/apis/Interfaces/MutableMap/)`` Tv> `` - a [` MutableMap `](/apis/Interfaces/MutableMap/) containing the values after a user-specified
  condition is applied to the keys and values of the current
  [` MutableMap `](/apis/Interfaces/MutableMap/).
<!-- HHAPIDOC -->
