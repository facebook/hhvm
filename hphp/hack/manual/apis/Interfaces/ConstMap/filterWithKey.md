
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstMap `](/apis/Interfaces/ConstMap/) containing the values of the current [` ConstMap `](/apis/Interfaces/ConstMap/) that
meet a supplied condition applied to its keys and values




``` Hack
public function filterWithKey(
  (function(Tk, Tv): bool) $fn,
): ConstMap<Tk, Tv>;
```




Only keys and values that meet a certain criteria are affected by a call to
[` filterWithKey() `](/apis/Interfaces/ConstMap/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Interfaces/ConstMap/mapWithKey/).




The keys associated with the current [` ConstMap `](/apis/Interfaces/ConstMap/) remain unchanged in the
returned [` ConstMap `](/apis/Interfaces/ConstMap/); the keys will be used in the filtering process only.




## Parameters




+ ` (function(Tk, Tv): bool) $fn ` - The callback containing the condition to apply to the current
  [` ConstMap `](/apis/Interfaces/ConstMap/) keys and values.




## Returns




* [` ConstMap<Tk, `](/apis/Interfaces/ConstMap/)`` Tv> `` - a [` ConstMap `](/apis/Interfaces/ConstMap/) containing the values after a user-specified
  condition is applied to the keys and values of the current
  [` ConstMap `](/apis/Interfaces/ConstMap/).
<!-- HHAPIDOC -->
