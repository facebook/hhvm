
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the values after an operation has been
applied to each value in the current [` Pair `](/docs/apis/Classes/HH/Pair/)




``` Hack
public function map<Tu>(
  (function(mixed): Tu) $callback,
): ImmVector<Tu>;
```




Every value in the current Pair is affected by a call to [` map() `](/docs/apis/Classes/HH/Pair/map/), unlike
[` filter() `](/docs/apis/Classes/HH/Pair/filter/) where only values that meet a certain criteria are affected.




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(mixed): Tu) $callback ` - The callback containing the operation to apply to the
  current [` Pair `](/docs/apis/Classes/HH/Pair/) values.




## Returns




- [` ImmVector<Tu> `](/docs/apis/Classes/HH/ImmVector/) - an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the values after a user-specified
  operation is applied.




## Examples




In this example the [` Pair `](/docs/apis/Classes/HH/Pair/)'s values are mapped to `` 0 `` if they're ``` NULL ```:




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
