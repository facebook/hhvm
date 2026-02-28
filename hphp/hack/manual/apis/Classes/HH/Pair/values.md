
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function values(): ImmVector<mixed>;
```




This method is interchangeable with [` toImmVector() `](/apis/Classes/HH/Pair/toImmVector/).




## Returns




+ [` ImmVector<mixed> `](/apis/Classes/HH/ImmVector/) - an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` Pair `](/apis/Classes/HH/Pair/).




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

$immutable_v = $p->values();

\var_dump($immutable_v);
```
<!-- HHAPIDOC -->
