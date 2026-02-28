
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a darray built from the values from this ImmSet, darray[val1 =>
val1, val2 => val2, ...]




``` Hack
public function toDArray(): darray<Tv, Tv>;
```




This maintains set-like semantics in darray[]
land: O(1) membership test with ` array_has_key($a['key']) ` and iteration
with `` foreach($a as $member) ``.




## Returns




+ ` darray `




## Examples




See [` Set::toDArray `](/apis/Classes/HH/Set/toDArray/#examples) for usage examples.
<!-- HHAPIDOC -->
