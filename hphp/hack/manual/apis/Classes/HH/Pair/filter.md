
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` Pair `](/apis/Classes/HH/Pair/) that
meet a supplied condition




``` Hack
public function filter(
  (function(mixed): bool) $callback,
): ImmVector<mixed>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/apis/Classes/HH/Pair/filter/), while all values are affected by a call to [` map() `](/apis/Classes/HH/Pair/map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(mixed): bool) $callback ` - The callback containing the condition to apply to the
  current [` Pair `](/apis/Classes/HH/Pair/) values.




## Returns




- [` ImmVector<mixed> `](/apis/Classes/HH/ImmVector/) - an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after a user-specified
  condition is applied.




## Examples




``` basic-usage.hack
$p = Pair {-1.5, null};

$v = $p->filter($value ==> $value !== null);
\var_dump($v);
```
<!-- HHAPIDOC -->
