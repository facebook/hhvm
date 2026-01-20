
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after an operation has been
applied to each value in the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function map<Tu>(
  (function(mixed): Tu) $callback,
): ImmVector<Tu>;
```




Every value in the current Pair is affected by a call to [` map() `](/apis/Classes/HH/Pair/map/), unlike
[` filter() `](/apis/Classes/HH/Pair/filter/) where only values that meet a certain criteria are affected.




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(mixed): Tu) $callback ` - The callback containing the operation to apply to the
  current [` Pair `](/apis/Classes/HH/Pair/) values.




## Returns




- [` ImmVector<Tu> `](/apis/Classes/HH/ImmVector/) - an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after a user-specified
  operation is applied.




## Examples




In this example the [` Pair `](/apis/Classes/HH/Pair/)'s values are mapped to `` 0 `` if they're ``` NULL ```:




``` basic-usage.hack
$p = Pair {null, -1.5};

$immutable_v = $p->map($value ==> {
  if ($value === null) {
    return 0;
  }
  return $value;
});
\var_dump($immutable_v);
```
<!-- HHAPIDOC -->
